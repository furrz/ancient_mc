#include <array>
#include <memory>
#include <optional>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ConVars.h"
#include "Input.h"
#include "Inventory.h"
#include "Level.h"
#include "LevelProcesses.h"
#include "LevelRenderer.h"
#include "Player.h"
#include "rng.h"

constexpr int WIDTH = 1024;
constexpr int HEIGHT = 768;
constexpr float ASPECT = WIDTH / static_cast<float>(HEIGHT);
constexpr glm::vec3 clearColor{ 0.5, 0.8, 1.0 };

class App
{
    GLFWwindow *window;
    std::unique_ptr<ConVars> conVars_;
    std::unique_ptr<leveldb::DB> db_;
    std::unique_ptr<BlockInfo> blockInfo;
    std::unique_ptr<Level> level;
    std::unique_ptr<LevelRenderer> levelRenderer;
    std::unique_ptr<Player> player;
    std::unique_ptr<Inventory> inventory;
    std::unique_ptr<LevelProcesses> processes;
    glm::dvec2 prevCursorPos{};
    bool breakBlock{}, placeBlock{};

    bool attrForceResetDatabase_ = false;

    static void mouseButtonCallback(GLFWwindow *window, const int button, const int action, const int)
    {
        const auto app = static_cast<App *>(glfwGetWindowUserPointer(window));
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            app->breakBlock = true;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
            app->placeBlock = true;
    }

public:
    App()
    {
        glfwInit();

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

        window = glfwCreateWindow(WIDTH, HEIGHT, "Game", nullptr, nullptr);
        Input::setWindow(window);
        glfwSetWindowUserPointer(window, this);
        glfwSetMouseButtonCallback(window, mouseButtonCallback);
        glfwMakeContextCurrent(window);

        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));

        /* initialize GL state */
        glEnable(GL_TEXTURE_2D);
        glShadeModel(GL_SMOOTH);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 0.0f);
        glClearDepth(1.0f);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (glfwRawMouseMotionSupported())
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

        conVars_ = std::make_unique<ConVars>();
        conVars_->load();

        conVars_->setupVar("app_force_reset_database",
            "Forcibly erase game.db at startup, erasing the world map and progress.",
            &attrForceResetDatabase_);

        // Force delete database if requested
        if (attrForceResetDatabase_) {
            std::cerr << "You have app_force_reset_database enabled. Deleting level.db." << std::endl;
            std::filesystem::remove_all("level.db");
        }

        // Load database
        leveldb::DB *db_tmp;
        leveldb::Options options;
        options.create_if_missing = true;
        leveldb::Status status = leveldb::DB::Open(options, "level.db", &db_tmp);
        db_ = std::unique_ptr<leveldb::DB>(db_tmp);

        blockInfo = std::make_unique<BlockInfo>();
        level = std::make_unique<Level>(db_.get(), conVars_.get(), 256, 256, 256, blockInfo.get());
        player = std::make_unique<Player>(db_.get(), conVars_.get(), level.get());
        inventory = std::make_unique<Inventory>(blockInfo.get());
        levelRenderer = std::make_unique<LevelRenderer>(conVars_.get(), level.get(), blockInfo.get());
        processes = std::make_unique<LevelProcesses>(conVars_.get(), blockInfo.get());

        conVars_->save();

        conVars_->validateNoPendingEntries();

        if (level->justGenerated())
            processes->runWorldGenProcesses(level.get());

    }

    void run()
    {
        double prevTime = glfwGetTime();
        double passedTimeInTicks = 0.0f;

        glfwGetCursorPos(window, &prevCursorPos.x, &prevCursorPos.y);

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            // Compute time
            const double newTime = glfwGetTime();
            const double deltaTime = newTime - prevTime;
            passedTimeInTicks += deltaTime * 60;
            const int ticks = static_cast<int>(passedTimeInTicks);
            passedTimeInTicks -= ticks;

            // Tick and Draw
            for (int i = 0; i < ticks; ++i) tick();
            render(static_cast<float>(passedTimeInTicks));

            if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                break;
            }

            prevTime = newTime;
        }
    }

    void tick() const
    {
        inventory->tick();
        player->tick();
        processes->tick(level.get(), player.get());
        level->swapTickedBlocks();
    }

    void render(const float delta)
    {
        // Compute mouse movement
        glm::dvec2 cursorPos;
        glfwGetCursorPos(window, &cursorPos.x, &cursorPos.y);
        const auto cursorDelta = cursorPos - prevCursorPos;
        prevCursorPos = cursorPos;

        player->turn({ cursorDelta.x, -cursorDelta.y });

        const auto hitResult = pick(delta);

        if (breakBlock) {
            breakBlock = false;

            if (hitResult) level->setTile(hitResult->pos, 0);
        }

        if (placeBlock) {
            placeBlock = false;

            if (hitResult) {
                // determine where we actually hit
                glm::ivec3 pos = hitResult->pos;
                switch (hitResult->f) {
                case 0: pos.y--;
                    break;
                case 1: pos.y++;
                    break;
                case 2: pos.z--;
                    break;
                case 3: pos.z++;
                    break;
                case 4: pos.x--;
                    break;
                case 5: pos.x++;
                    break;
                default: break;
                }

                level->setTile(pos, inventory->getBlockId());
            }
        }

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        setupCamera(delta);

        levelRenderer->handleDirtyRegions();

        glEnable(GL_CULL_FACE);
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, 40.0f);
        glFogf(GL_FOG_END, 80.0f);
        glFogf(GL_FOG_DENSITY, 0.5f);
        glFogfv(GL_FOG_COLOR, glm::value_ptr(clearColor));
        glDisable(GL_BLEND);

        // Draw all render layers
        levelRenderer->render(player->pos());


        //Draw hit result
        if (hitResult) {
            glDisable(GL_TEXTURE_2D);
            levelRenderer->renderHit(hitResult.value());
            glEnable(GL_TEXTURE_2D);
        }

        inventory->render();
        glfwSwapBuffers(window);
    }

    void setupCamera(const float delta) const
    {
        glMatrixMode(GL_PROJECTION);
        const glm::mat4 perspective = glm::perspective(
            glm::radians(70.0f),
            ASPECT, 0.05f, 128.0f);
        glLoadMatrixf(glm::value_ptr(perspective));

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        moveCameraToPlayer(delta);
    }

    // set up the projection matrices for picking
    void setupPickCamera(const float delta) const
    {
        glm::ivec4 viewportBuffer;
        glGetIntegerv(GL_VIEWPORT, glm::value_ptr(viewportBuffer));

        glMatrixMode(GL_PROJECTION);

        float x_scale, y_scale;
        glfwGetWindowContentScale(window, &x_scale, &y_scale);

        const auto pick = glm::pickMatrix(glm::vec2{
                                              WIDTH * x_scale / 2.0f,
                                              HEIGHT * y_scale / 2.0f
                                          }, glm::vec2{ 5.0f, 5.0f }, viewportBuffer);

        const auto perspective = glm::perspective(
            glm::radians(70.0f),
            ASPECT, 0.05f, 128.0f);

        glLoadMatrixf(glm::value_ptr(pick * perspective));

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        moveCameraToPlayer(delta);
    }

    // pick a block
    [[nodiscard]] std::optional<HitResult> pick(const float delta) const
    {
        std::array<unsigned, 2000> selectBuffer{};
        glSelectBuffer(selectBuffer.size(), selectBuffer.data());
        glRenderMode(GL_SELECT);

        setupPickCamera(delta);
        levelRenderer->pick(player->box());

        const int hits = glRenderMode(GL_RENDER);

        long closest{};
        std::array<int, 10> names{};
        int hitNameCount = 0;

        int cursor = 0;
        for (int i = 0; i < hits; i++) {
            const int nameCount = static_cast<int>(selectBuffer[cursor++]);
            const long minZ = static_cast<long>(selectBuffer[cursor++]);
            cursor++;

            const long dist = minZ;
            int j;
            if (dist >= closest && i != 0) {
                cursor += nameCount;
            } else {
                closest = dist;
                hitNameCount = nameCount;

                for (j = 0; j < nameCount; ++j) {
                    names[j] = static_cast<int>(selectBuffer[cursor++]);
                }
            }
        }

        if (hitNameCount > 0)
            return HitResult(
                { names[0], names[1], names[2] },
                names[3], names[4]
            );

        return std::nullopt;
    }

    void moveCameraToPlayer(const float delta) const
    {
        glTranslatef(0.0F, 0.0F, -0.3F);
        glRotatef(player->rot().x, 1.0F, 0.0F, 0.0F);
        glRotatef(player->rot().y, 0.0F, 1.0F, 0.0F);
        const auto pos = player->posInterpolated(delta);
        glTranslatef(-pos.x, -pos.y, -pos.z);
    }

    ~App()
    {
        level->save();
        player->save();
        conVars_->save();
        glfwTerminate();
    }
};

int main()
{
    RNG::seed();

    App app;
    app.run();

    return 0;
}
