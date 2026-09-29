#pragma once

#include <juce_graphics/juce_graphics.h>

namespace raiu::ui {

// 有機的・未来的なダークパレット。セクションごとにアクセントカラーを持たせ、
// 「触っている場所」が色で直感的にわかるようにする。
namespace Palette {
inline const juce::Colour bgDeep{0xff0a0c12};
inline const juce::Colour bgVoid{0xff05060a};
inline const juce::Colour panel{0xff141824};
inline const juce::Colour panelRaised{0xff1c2130};
inline const juce::Colour hairline{0xff2a3040};

inline const juce::Colour textPrimary{0xfff2f4f8};
inline const juce::Colour textMuted{0xff8d97ac};
inline const juce::Colour textFaint{0xff5a6278};

// FM / Operators — 銅・琥珀（電気の熱）
inline const juce::Colour fm{0xffe8a24c};
inline const juce::Colour fmDim{0xff8a6234};
// Nature Osc — 生命感のあるティール
inline const juce::Colour nature{0xff45d9a8};
inline const juce::Colour natureDim{0xff2a7d64};
// Texture（環境音）— 夜空の青
inline const juce::Colour texture{0xff5ba3e0};
inline const juce::Colour textureDim{0xff35597a};
// Interaction（呼吸）— オーロラ紫
inline const juce::Colour interaction{0xffb479ea};
inline const juce::Colour interactionDim{0xff6b4a8a};
// FX / Global — 銀青
inline const juce::Colour fx{0xff93a8d0};
inline const juce::Colour fxDim{0xff56617a};
} // namespace Palette

} // namespace raiu::ui
