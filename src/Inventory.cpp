#include <glad/glad.h>
#include "Inventory.h"

void Inventory::tick()
{
    // TODO inventory tick
}

uint8_t Inventory::getBlockId() const
{
    return slots_[slot_];
}

void Inventory::render()
{
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, 1024, 0, 768, -1.0f, 1.0f);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_CULL_FACE);
    glBegin(GL_QUADS);

    constexpr int ICON_SIZE = 50;
    constexpr int ICON_SPACING = 10;
    constexpr float ICONS_MARGIN = 10.0f;

    for (int i = 0; i < slots_.size(); i++) {
        float x = i * ICON_SIZE + i * ICON_SPACING + ICONS_MARGIN;
        float color = (slot_ == i) ? 1.0f : 0.8f;

        float u0 = (float)(slots_[i] - 1) / 16.0F;
        float u1 = u0 + 0.0624375F;
        float v0 = 0.0F;
        float v1 = v0 + 0.0624375F;

        glColor4f(color, color, color, color);
        glTexCoord2d(u0, v0);
        glVertex2f(x, ICONS_MARGIN);
        glTexCoord2d(u0, v1);
        glVertex2f(x, ICONS_MARGIN + ICON_SIZE);
        glTexCoord2d(u1, v1);
        glVertex2f(x + ICON_SIZE, ICONS_MARGIN + ICON_SIZE);
        glTexCoord2d(u1, v0);
        glVertex2f(x + ICON_SIZE, ICONS_MARGIN);
    }
    glEnd();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
}
