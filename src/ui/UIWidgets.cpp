#include "ui/UIWidgets.hpp"
#include "ui/UIUtils.hpp"
#include <algorithm>
#include <cmath>

namespace ui {

bool UIRect::contains(float px, float py) const {
    return px >= x && px <= (x + w) && py >= y && py <= (y + h);
}

void UIButton::draw(UIRenderer& renderer,
                    const UIRect& rect,
                    const renderer::Texture& texIdle,
                    const renderer::Texture& texHover,
                    const std::string& label,
                    float textScale,
                    bool isHovered,
                    const glm::vec4& textColor,
                    float cornerSize) {
    drawTinted(renderer, rect, texIdle, texHover, label, textScale, isHovered,
               textColor, glm::vec4(1.0f), cornerSize);
}

void UIButton::drawTinted(UIRenderer& renderer,
                          const UIRect& rect,
                          const renderer::Texture& texIdle,
                          const renderer::Texture& texHover,
                          const std::string& label,
                          float textScale,
                          bool isHovered,
                          const glm::vec4& textColor,
                          const glm::vec4& bgTint,
                          float cornerSize) {
    const renderer::Texture& tex = (isHovered && texHover.isLoaded()) ? texHover : texIdle;

    if (tex.isLoaded()) {
        if (cornerSize > 0.0f) {
            renderer.drawNineSlice(tex, rect.x, rect.y, rect.w, rect.h, cornerSize, cornerSize, bgTint);
        } else {
            renderer.drawSprite(tex, rect.x, rect.y, rect.w, rect.h, bgTint);
        }
    } else {
        glm::vec4 fallbackColor = isHovered ? glm::vec4(0.25f, 0.45f, 0.80f, 0.95f)
                                            : glm::vec4(0.15f, 0.20f, 0.30f, 0.95f);
        renderer.drawRect(rect.x, rect.y, rect.w, rect.h, fallbackColor * bgTint);
    }

    if (!label.empty()) {
        std::string cleanLabel = sanitizeText(label);
        if (isHovered) {
            renderer.drawTextCentered(rect.x, rect.y, rect.w, rect.h, cleanLabel, textScale, textColor);
        } else {
            renderer.drawTextCenteredWithShadow(rect.x, rect.y, rect.w, rect.h, cleanLabel, textScale, textColor);
        }
    }
}

void UIButton::drawIconButton(UIRenderer& renderer,
                              const UIRect& rect,
                              const renderer::Texture& iconTex,
                              bool isHovered,
                              const glm::vec4& idleTint,
                              const glm::vec4& hoverTint,
                              float hoverScale) {
    if (!iconTex.isLoaded()) return;

    float scale = isHovered ? hoverScale : 1.0f;
    float drawW = rect.w * scale;
    float drawH = rect.h * scale;
    float drawX = rect.x - (drawW - rect.w) * 0.5f;
    float drawY = rect.y - (drawH - rect.h) * 0.5f;
    renderer.drawSprite(iconTex, drawX, drawY, drawW, drawH, isHovered ? hoverTint : idleTint);
}

void UIButton::drawArrowIndicator(UIRenderer& renderer,
                                  const renderer::Texture& arrowTex,
                                  float x,
                                  float y,
                                  float size,
                                  float animTimer,
                                  float bounceAmp) {
    if (!arrowTex.isLoaded()) return;
    float bounce = std::sin(animTimer * 6.0f) * bounceAmp;
    renderer.drawSprite(arrowTex, x + bounce, y, size, size);
}

void UIPanel::drawFramedPanel(UIRenderer& renderer,
                              const UIRect& rect,
                              const renderer::Texture& frameTex,
                              float cornerSize,
                              const glm::vec4& bgTint,
                              const glm::vec4& borderTint) {
    renderer.drawRect(rect.x + 2.0f, rect.y + 2.0f,
                      std::max(0.0f, rect.w - 4.0f), std::max(0.0f, rect.h - 4.0f), bgTint);

    if (frameTex.isLoaded()) {
        renderer.drawNineSlice(frameTex, rect.x, rect.y, rect.w, rect.h, cornerSize, cornerSize);
    }

    renderer.drawBorder(rect.x, rect.y, rect.w, rect.h, 2.0f, borderTint);
}

void UIPanel::drawBannerBox(UIRenderer& renderer,
                            const UIRect& rect,
                            const renderer::Texture& innerTex,
                            const std::string& text,
                            float textScale,
                            const glm::vec4& textColor,
                            const glm::vec4& borderColor,
                            const glm::vec4& innerTint,
                            float cornerSize) {
    if (innerTex.isLoaded()) {
        renderer.drawNineSlice(innerTex, rect.x, rect.y, rect.w, rect.h, cornerSize, cornerSize, innerTint);
    } else {
        renderer.drawRect(rect.x, rect.y, rect.w, rect.h, innerTint);
    }
    if (borderColor.a > 0.0f) {
        renderer.drawBorder(rect.x, rect.y, rect.w, rect.h, 1.5f, borderColor);
    }
    if (!text.empty()) {
        renderer.drawTextCentered(rect.x, rect.y, rect.w, rect.h, sanitizeText(text), textScale, textColor);
    }
}

void UIProgressBar::drawStatBar(UIRenderer& renderer,
                                const UIRect& rect,
                                const std::string& label,
                                const std::string& valueStr,
                                float ratio,
                                const glm::vec4& fillColor,
                                const renderer::Texture& barBgL,
                                const renderer::Texture& barBgM,
                                const renderer::Texture& barBgR,
                                const renderer::Texture& barFillL,
                                const renderer::Texture& barFillM,
                                const renderer::Texture& barFillR) {
    std::string cleanLabel = sanitizeText(label);
    std::string cleanVal = sanitizeText(valueStr);

    renderer.drawText(rect.x, rect.y, cleanLabel, 1.4f, glm::vec4(0.85f, 0.90f, 0.96f, 1.0f));
    float valW = renderer.getTextWidth(cleanVal, 1.4f);
    renderer.drawText(rect.x + rect.w - valW, rect.y, cleanVal, 1.4f, fillColor);

    float barY = rect.y + 16.0f;
    float barH = std::max(12.0f, rect.h - 16.0f);

    bool singleBg = (&barBgL == &barBgM && &barBgM == &barBgR);
    if (singleBg && barBgL.isLoaded()) {
        renderer.drawThreeSliceHorizontal(barBgL, rect.x, barY, rect.w, barH, 8.0f,
                                          glm::vec4(0.30f, 0.35f, 0.45f, 0.95f));
    } else if (!singleBg && barBgL.isLoaded() && barBgM.isLoaded() && barBgR.isLoaded()) {
        renderer.drawThreeSliceHorizontal(barBgL, barBgM, barBgR, rect.x, barY, rect.w, barH, 8.0f,
                                          glm::vec4(0.30f, 0.35f, 0.45f, 0.95f));
    } else {
        renderer.drawRect(rect.x, barY, rect.w, rect.h, glm::vec4(0.18f, 0.20f, 0.25f, 0.95f));
    }

    float clampedRatio = std::clamp(ratio, 0.05f, 1.0f);
    bool singleFill = (&barFillL == &barFillM && &barFillM == &barFillR);
    if (singleFill && barFillL.isLoaded()) {
        renderer.drawSpriteProgress(barFillL, rect.x + 2.0f, barY + 2.0f, rect.w - 4.0f, barH - 4.0f,
                                    clampedRatio, true, fillColor);
    } else if (!singleFill && barFillL.isLoaded() && barFillM.isLoaded() && barFillR.isLoaded()) {
        renderer.drawThreeSliceProgress(barFillL, barFillM, barFillR,
                                        rect.x + 2.0f, barY + 2.0f, rect.w - 4.0f, barH - 4.0f,
                                        clampedRatio, 8.0f, fillColor);
    } else {
        renderer.drawRect(rect.x + 2.0f, barY + 2.0f, clampedRatio * (rect.w - 4.0f), barH - 4.0f, fillColor);
    }

    for (int t = 1; t <= 4; ++t) {
        float tickX = rect.x + (rect.w * 0.20f * static_cast<float>(t));
        renderer.drawRect(tickX - 1.0f, barY + 1.0f, 2.0f, barH - 2.0f,
                          glm::vec4(0.08f, 0.10f, 0.15f, 0.65f));
    }

    renderer.drawBorder(rect.x, barY, rect.w, barH, 1.0f, glm::vec4(0.4f, 0.5f, 0.65f, 0.5f));
}

void UIProgressBar::drawStatBar(UIRenderer& renderer,
                                const UIRect& rect,
                                const std::string& label,
                                const std::string& valueStr,
                                float ratio,
                                const glm::vec4& fillColor,
                                const renderer::Texture& barBg,
                                const renderer::Texture& barFill) {
    drawStatBar(renderer, rect, label, valueStr, ratio, fillColor,
                barBg, barBg, barBg, barFill, barFill, barFill);
}

int UITabBar::hitTest(float startX,
                      float y,
                      float tabW,
                      float tabH,
                      float spacing,
                      int tabCount,
                      float mouseX,
                      float mouseY) {
    for (int i = 0; i < tabCount; ++i) {
        UIRect tabRect{startX + static_cast<float>(i) * (tabW + spacing), y, tabW, tabH};
        if (tabRect.contains(mouseX, mouseY)) {
            return i;
        }
    }
    return -1;
}

int UITabBar::draw(UIRenderer& renderer,
                   float startX,
                   float y,
                   float tabW,
                   float tabH,
                   float spacing,
                   const std::vector<std::string>& labels,
                   int activeIndex,
                   const renderer::Texture& texIdle,
                   const renderer::Texture& texActive,
                   const renderer::Texture& texHover,
                   const renderer::Texture* arrowDownTex,
                   float mouseX,
                   float mouseY,
                   bool mouseClicked) {
    int clickedIdx = -1;
    int count = static_cast<int>(labels.size());

    for (int i = 0; i < count; ++i) {
        UIRect tabRect{startX + static_cast<float>(i) * (tabW + spacing), y, tabW, tabH};
        bool isActive = (i == activeIndex);
        bool isHover = tabRect.contains(mouseX, mouseY);
        if (isHover && mouseClicked) {
            clickedIdx = i;
        }

        std::string cleanText = sanitizeText(labels[i]);

        if (isActive) {
            renderer.drawNineSlice(texActive.isLoaded() ? texActive : texIdle,
                                   tabRect.x, tabRect.y, tabRect.w, tabRect.h, 12.0f, 12.0f);
            renderer.drawBorder(tabRect.x - 2.0f, tabRect.y - 2.0f, tabRect.w + 4.0f, tabRect.h + 4.0f, 3.0f,
                                glm::vec4(1.0f, 0.85f, 0.15f, 1.0f));
            if (arrowDownTex && arrowDownTex->isLoaded()) {
                renderer.drawSprite(*arrowDownTex,
                                    tabRect.x + (tabRect.w - 24.0f) * 0.5f, tabRect.y + tabRect.h + 3.0f,
                                    24.0f, 20.0f, glm::vec4(1.0f, 0.85f, 0.15f, 1.0f));
            }
            renderer.drawTextCentered(tabRect.x, tabRect.y, tabRect.w, tabRect.h, cleanText, 1.9f,
                                      glm::vec4(0.10f, 0.12f, 0.18f, 1.0f));
        } else if (isHover) {
            renderer.drawNineSlice(texHover.isLoaded() ? texHover : texIdle,
                                   tabRect.x, tabRect.y, tabRect.w, tabRect.h, 12.0f, 12.0f,
                                   glm::vec4(1.0f, 1.0f, 0.8f, 1.0f));
            renderer.drawBorder(tabRect.x, tabRect.y, tabRect.w, tabRect.h, 1.5f,
                                glm::vec4(0.9f, 0.9f, 0.5f, 0.8f));
            renderer.drawTextCentered(tabRect.x, tabRect.y, tabRect.w, tabRect.h, cleanText, 1.8f,
                                      glm::vec4(1.0f, 0.95f, 0.7f, 1.0f));
        } else {
            renderer.drawNineSlice(texIdle, tabRect.x, tabRect.y, tabRect.w, tabRect.h, 12.0f, 12.0f);
            renderer.drawTextCentered(tabRect.x, tabRect.y, tabRect.w, tabRect.h, cleanText, 1.8f,
                                      glm::vec4(0.85f, 0.92f, 1.0f, 0.9f));
        }
    }

    return clickedIdx;
}

} // namespace ui
