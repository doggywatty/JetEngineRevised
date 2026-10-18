// source/game/interfaces/menus/subpages/OptionsAboutPage.cpp
#include "OptionsAboutPage.h"
#include "game/interfaces/menus/Menu_Base.h"
#include "sound/soundmgr.h"
#include "version.h"
#include "game/Game.h"
#include "game/gamemanagers/COPPAMgr.h"
#include "game/gamemanagers/FontInfoMgr.h"
#include "game/interfaces/interfaceobjects/InterfaceButton.h"
#include "game/interfaces/interfaceobjects/InterfaceText.h"
#include "game/interfaces/menus/MenuMgr.h"
#include "game/interfaces/menus/Menu_Options.h"
#include "game/utils/GameUtils.h"
#include "localization/babel.h"
#include "localization/stringmgr.h"
#include "sprites/fontmgr.h"

// @0x3bde04 (static init)
const jet::String OptionsAboutPage::k_startText("STR_ABOUT_START");
const jet::String OptionsAboutPage::k_versionText("STR_ABOUT_VERSION");
const jet::String OptionsAboutPage::k_startNumberText("STR_ABOUT_001");
const jet::String OptionsAboutPage::k_CMCCText("STR_ABOUT_CMCC");
const jet::String OptionsAboutPage::k_CTText("STR_ABOUT_CT");
const jet::String OptionsAboutPage::k_OperatorText("STR_ABOUT_499");

// k_lineSpacingY (@0xc89da8)
static const float k_lineSpacingY = 3.0f;

// the style a credits line is drawn with: the defaults with the palette's colours
static jet::text2::Texter::Style LineStyle(int pal) {
    jet::text2::Texter::Style s;
    s.primaryColor = (unsigned)InterfaceObject::GetPaletteColor(pal) | 0xff000000u;
    s.decorationColor = (unsigned)InterfaceObject::GetPaletteOutlineColor(pal) | 0xff000000u;
    return s;
}

// the key after a credits line: START, VERSION, 001 ... 499, then none
static jet::String NextKey(const jet::String &key, const jet::String &startText) {
    if (key.Equals(startText)) return OptionsAboutPage::k_versionText;
    if (key.Equals(OptionsAboutPage::k_versionText)) return OptionsAboutPage::k_startNumberText;
    if (key.Equals(OptionsAboutPage::k_OperatorText)) return jet::String("");
    return GameUtils::IncreaseKey(key, 1);
}

// @0x4a094c: the contact button only where the country rules ask for it
OptionsAboutPage::OptionsAboutPage()
    : BasicPage(jet::String("Options_About")), m_textArea(0), m_privacyPolicyButton(0), m_customerCareButton(0), m_termsButton(0), m_showContact(true) {
    m_textArea = GetUIText(jet::String("AboutText"));
    m_privacyPolicyButton = GetUIButton(jet::String("Privacy_button"));
    m_customerCareButton = GetUIButton(jet::String("Contact_button"));
    m_termsButton = GetUIButton(jet::String("Terms_button"));
    m_showContact = COPPAMgr::GetInstance()->MustShowContactSection();
    m_customerCareButton->SetVisible(m_showContact);
}

OptionsAboutPage::~OptionsAboutPage() {}

// @0x4a0cec: privacy and terms open the web page (offline an info popup says so), contact opens its menu
void OptionsAboutPage::Update(int dt) {
    BasicPage::Update(dt);
    MenuMgr *mm = MenuMgr::GetInstance();
    int n = mm->ACTION_GetNumReleasedButtons();
    for (int i = 0; i < n; i++) {
        InterfaceButton *b = mm->ACTION_GetReleasedButton(i);
        if (b == m_privacyPolicyButton || b == m_termsButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            if (Game::GetInstance()->AreOnlineServicesReady()) {
                // nativeOpenBrowser("https://201205igp.gameloft.com/redir/?from=" + game code + "&op=GAND&ctg=PRIVACY" / "TERMS"):
                // an Android call, not in this build
            }
            // else PopupMgr::PushInfoPopup(POPUP_NO_INTERNET): PopupMgr is not ported
            return;
        }
        if (b == m_customerCareButton) {
            SoundMgr::GetInstance()->Play(Menu_Base::k_sfx_menu_click, 0);
            mm->PushMenu(Menu_OptionsContact::k_menuName);
            return;
        }
    }
}

// @0x4a0fc4: the page, then the credits below the buttons, the lines inside the list drawn centered
void OptionsAboutPage::Render(const jet::scene::Camera &cam, jet::video::Painter &painter) {
    if (!IsVisible()) return;
    BasicPage::Render(cam, painter);
    float y = m_textArea->GetSize().y * 2.0f + GetOffset().y + m_privacyPolicyButton->GetSize().y + m_termsButton->GetSize().y;
    if (m_showContact) y += m_customerCareButton->GetSize().y;
    int font = 0, pal = 0;
    float maxWidth = m_textArea->GetSize().x;
    FontMgr *fm = FontMgr::GetInstance();
    InterfaceObject *list = m_parent;
    float x = list->GetPos().x + list->GetSize().x * 0.5f;
    math::vec2<float> lp = list->GetPos(), ls = list->GetSize();
    math::rect<float> clip(lp.x, lp.y, ls.x + lp.x, ls.y + lp.y);
    babel::StringMgr *sm = babel::Babel::GetInstance()->GetStringMgr();
    jet::String key = m_startText;
    do {
        jet::String text = sm->Get(key);
        ParseFontTypeAndPalette(text, font, pal);
        const jet::String &fontName = FontInfoMgr::GetFontName((unsigned)font);
        jet::text2::Texter::Style style = LineStyle(pal);
        if (key.Equals(k_versionText)) text = jet::String::Format(text.c_str(), (despicableme::Version::GetGameVersion() + "k").c_str());
        math::vec2<float> sz = fm->GetTextSize(text.c_str(), fontName, maxWidth, style);
        float next = sz.y + k_lineSpacingY + y;
        if (next >= clip.upperLeftCorner.y && y < clip.lowerRightCorner.y) {
            math::mat4<float> transform;
            transform.setIdentity();
            fm->DrawString(text.c_str(), x, y, safe_enum<jet::text2::Texter::AlignmentDef, jet::text2::Texter::AlignmentDef::type>(jet::text2::Texter::AlignmentDef::TOP_CENTER),
                           fontName, style, 1.0f, maxWidth, false, clip, transform);
        }
        y = next;
        key = NextKey(key, m_startText);
    } while (sm->Exists(key));
}

// @0x4a1550: the buttons go under the title, the page is as tall as the buttons and all the credits lines
void OptionsAboutPage::Refresh() {
    m_startText = k_startText;
    int font = 0, pal = 0;
    float width = m_textArea->GetSize().x, textH = m_textArea->GetSize().y;
    float height = textH + textH + m_privacyPolicyButton->GetSize().y + m_termsButton->GetSize().y;
    float y = textH;
    if (m_showContact) height += m_customerCareButton->GetSize().y;
    else y -= m_customerCareButton->GetSize().y;
    m_customerCareButton->SetOffset(math::vec2<float>(0, y));
    m_privacyPolicyButton->SetOffset(math::vec2<float>(0, y));
    m_termsButton->SetOffset(math::vec2<float>(0, y));
    FontMgr *fm = FontMgr::GetInstance();
    babel::StringMgr *sm = babel::Babel::GetInstance()->GetStringMgr();
    jet::String key = m_startText;
    do {
        jet::String text = sm->Get(key);
        ParseFontTypeAndPalette(text, font, pal);
        if (key.Equals(k_versionText)) text = jet::String::Format(text.c_str(), despicableme::Version::GetGameVersion().c_str());
        const jet::String &fontName = FontInfoMgr::GetFontName((unsigned)font);
        height += fm->GetTextSize(text.c_str(), fontName, width, LineStyle(pal)).y + k_lineSpacingY;
        key = NextKey(key, m_startText);
    } while (sm->Exists(key));
    SetSize(math::vec2<float>(width, height));
}

// @0x4a19a4: "#Ffp" in front of the text gives the font f and the palette p, else the text area's
void OptionsAboutPage::ParseFontTypeAndPalette(jet::String &text, int &font, int &pal) {
    if (text.size() > 3) {
        const char *s = text.c_str();
        if (s[0] == '#' && s[1] == 'F') {
            font = s[2] - '0';
            pal = text.c_str()[3] - '0';
            text = text.substr(4);
            return;
        }
    }
    font = m_textArea->GetFontIdx();
    pal = m_textArea->GetPalIdx();
}
