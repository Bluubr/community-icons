#pragma once

#include <Geode/Geode.hpp>
#include <Geode/ui/TextInput.hpp>
#include <Geode/utils/web.hpp>
#include <Geode/utils/async.hpp>
#include <chrono>
#include <algorithm>
#include <cctype>
#include "FirebaseAuth.hpp"

using namespace geode::prelude;

class SubmitPackPopup : public geode::Popup {
protected:
    TextInput*     m_nameInput         = nullptr;
    TextInput*     m_authorInput       = nullptr;
    TextInput*     m_gamemodeInput     = nullptr;
    TextInput*     m_imageUrlInput     = nullptr;
    TextInput*     m_plistUrlInput     = nullptr;
    TextInput*     m_graphicsTypeInput = nullptr;
    CCLabelBMFont* m_statusLabel       = nullptr;

    std::optional<arc::TaskHandle<void>> m_submitHandle;

    static constexpr int64_t     COOLDOWN_SECS = 3600;
    static constexpr const char* COOLDOWN_KEY  = "last_pack_submit_epoch";

    static std::vector<std::string> const& bannedWords() {
        static const std::vector<std::string> words = {
            "2g1c","2 girls 1 cup","acrotomophilia","alabama hot pocket",
            "alaskan pipeline","anal","anilingus","anus","apeshit","arsehole",
            "ass","asshole","assmunch","auto erotic","autoerotic","babeland",
            "baby batter","baby juice","ball gag","ball gravy","ball kicking",
            "ball licking","ball sack","ball sucking","bangbros","bangbus",
            "bareback","barely legal","barenaked","bastard","bastardo",
            "bastinado","bbw","bdsm","beaner","beaners","beaver cleaver",
            "beaver lips","beastiality","bestiality","big black","big breasts",
            "big knockers","big tits","bimbos","birdlock","bitch","bitches",
            "black cock","blonde action","blonde on blonde action","blowjob",
            "blow job","blow your load","blue waffle","blumpkin","bollocks",
            "bondage","boner","boob","boobs","booty call","brown showers",
            "brunette action","bukkake","bulldyke","bullet vibe","bullshit",
            "bung hole","bunghole","busty","butt","buttcheeks","butthole",
            "camel toe","camgirl","camslut","camwhore","carpet muncher",
            "carpetmuncher","chocolate rosebuds","cialis","circlejerk",
            "cleveland steamer","clit","clitoris","clover clamps","clusterfuck",
            "cock","cocks","coprolagnia","coprophilia","cornhole","coon","coons",
            "creampie","cum","cumming","cumshot","cumshots","cunnilingus","cunt",
            "darkie","date rape","daterape","deep throat","deepthroat",
            "dendrophilia","dick","dildo","dingleberry","dingleberries",
            "dirty pillows","dirty sanchez","doggie style","doggiestyle",
            "doggy style","doggystyle","dog style","dolcett","domination",
            "dominatrix","dommes","donkey punch","double dong",
            "double penetration","dp action","dry hump","dvda","eat my ass",
            "ecchi","ejaculation","erotic","erotism","escort","eunuch","fag",
            "faggot","fecal","felch","fellatio","feltch","female squirting",
            "femdom","figging","fingerbang","fingering","fisting","foot fetish",
            "footjob","frotting","fuck","fuck buttons","fuckin","fucking",
            "fucktards","fudge packer","fudgepacker","futanari","gangbang",
            "gang bang","gay sex","genitals","giant cock","girl on",
            "girl on top","girls gone wild","goatcx","goatse","god damn",
            "gokkun","golden shower","goodpoop","goo girl","goregasm","grope",
            "group sex","g-spot","guro","hand job","handjob","hard core",
            "hardcore","hentai","homoerotic","honkey","hooker","horny",
            "hot carl","hot chick","how to kill","how to murder","huge fat",
            "humping","incest","intercourse","jack off","jail bait","jailbait",
            "jelly donut","jerk off","jigaboo","jiggaboo","jiggerboo","jizz",
            "juggs","kike","kinbaku","kinkster","kinky","knobbing",
            "leather restraint","leather straight jacket","lemon party","livesex",
            "lolita","lovemaking","make me come","male squirting","masturbate",
            "masturbating","masturbation","menage a trois","milf",
            "missionary position","mong","motherfucker","mound of venus",
            "mr hands","muff diver","muffdiving","nambla","nawashi","negro",
            "neonazi","nigga","nigger","nig nog","nimphomania","nipple",
            "nipples","nsfw","nsfw images","nude","nudity","nutten","nympho",
            "nymphomania","octopussy","omorashi","one cup two girls",
            "one guy one jar","orgasm","orgy","paedophile","paki","panties",
            "panty","pedobear","pedophile","pegging","penis","phone sex",
            "piece of shit","pikey","pissing","piss pig","pisspig","playboy",
            "pleasure chest","pole smoker","ponyplay","poof","poon","poontang",
            "punany","poop chute","poopchute","porn","porno","pornography",
            "prince albert piercing","pthc","pubes","pussy","queaf","queef",
            "quim","raghead","raging boner","rape","raping","rapist","rectum",
            "reverse cowgirl","rimjob","rimming","rosy palm",
            "rosy palm and her 5 sisters","rusty trombone","sadism","santorum",
            "scat","schlong","scissoring","semen","sex","sexcam","sexo","sexy",
            "sexual","sexually","sexuality","shaved beaver","shaved pussy",
            "shemale","shibari","shit","shitblimp","shitty","shota","shrimping",
            "skeet","slanteye","slut","s&m","smut","snatch","snowballing",
            "sodomize","sodomy","spastic","spic","splooge","splooge moose",
            "spooge","spread legs","spunk","strap on","strapon","strappado",
            "strip club","style doggy","suck","sucks","suicide girls",
            "sultry women","swastika","swinger","tainted love","taste my",
            "tea bagging","threesome","throating","thumbzilla","tied up",
            "tight white","tit","tits","titties","titty","tongue in a",
            "topless","tosser","towelhead","tranny","tribadism","tub girl",
            "tubgirl","tushy","twat","twink","twinkie","two girls one cup",
            "undressing","upskirt","urethra play","urophilia","vagina",
            "venus mound","viagra","vibrator","violet wand","vorarephilia",
            "voyeur","voyeurweb","voyuer","vulva","wank","wetback","wet dream",
            "white power","whore","worldsex","wrapping men","wrinkled starfish",
            "xx","xxx","yaoi","yellow showers","yiffy","zoophilia"
        };
        return words;
    }

    static bool containsBanned(std::string const& text) {
        std::string lower = text;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return std::tolower(c); });
        for (auto const& w : bannedWords())
            if (lower.find(w) != std::string::npos) return true;
        return false;
    }

    static int64_t nowSecs() {
        return static_cast<int64_t>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now().time_since_epoch()).count());
    }

    static bool onCooldown(int& secondsLeft) {
        int64_t last    = Mod::get()->getSavedValue<int64_t>(COOLDOWN_KEY, 0LL);
        int64_t elapsed = nowSecs() - last;
        if (elapsed < COOLDOWN_SECS) {
            secondsLeft = static_cast<int>(COOLDOWN_SECS - elapsed);
            return true;
        }
        return false;
    }

    static void recordSubmit() {
        Mod::get()->setSavedValue<int64_t>(COOLDOWN_KEY, nowSecs());
    }

    static void appendApiKey(std::string& url) {
        auto k = Mod::get()->getSettingValue<std::string>("firebase-api-key");
        if (!k.empty())
            url += (url.find('?') == std::string::npos ? "?" : "&") + ("key=" + k);
    }

    static bool isHttpsUrl(std::string const& url) {
        if (url.size() < 8) return false;
        std::string prefix = url.substr(0, 8);
        std::transform(prefix.begin(), prefix.end(), prefix.begin(),
            [](unsigned char c) { return std::tolower(c); });
        return prefix == "https://";
    }

    static std::string escJson(std::string const& s) {
        std::string out;
        out.reserve(s.size() + 8);
        for (unsigned char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n";  break;
                case '\r': out += "\\r";  break;
                case '\t': out += "\\t";  break;
                default:
                    if (c < 0x20) {
                        char b[8];
                        snprintf(b, sizeof(b), "\\u%04x", static_cast<unsigned int>(c));
                        out += b;
                    } else {
                        out += static_cast<char>(c);
                    }
            }
        }
        return out;
    }

    static std::string buildDoc(
        std::string const& name, std::string const& author,
        std::string const& gamemode, std::string const& imageUrl,
        std::string const& plistUrl, std::string const& graphicsType,
        std::string const& submittedBy)
    {
        return std::string("{\"fields\":{")
            + "\"name\":{\"stringValue\":\""        + escJson(name)        + "\"},"
            + "\"author\":{\"stringValue\":\""      + escJson(author)      + "\"},"
            + "\"gamemode\":{\"stringValue\":\""    + escJson(gamemode)    + "\"},"
            + "\"imageUrl\":{\"stringValue\":\""    + escJson(imageUrl)    + "\"},"
            + "\"plistUrl\":{\"stringValue\":\""    + escJson(plistUrl)    + "\"},"
            + "\"graphicsType\":{\"stringValue\":\"" + escJson(graphicsType) + "\"},"
            + "\"submittedBy\":{\"stringValue\":\"" + escJson(submittedBy) + "\"}"
            + "}}";
    }

    bool init() {
        if (!Popup::init(420.f, 380.f)) return false;
        this->setTitle("Submit Icon Pack");
        m_bgSprite->setColor({30, 25, 45});

        auto winSize = m_mainLayer->getContentSize();
        const float labelX = 118.f;
        const float inputX = winSize.width / 2.f + 20.f;
        const float inputW = 200.f;
        const float startY = winSize.height - 55.f;
        const float rowH   = 38.f;

        auto addRow = [&](const char* lbl, float y, const char* placeholder) -> TextInput* {
            auto l = CCLabelBMFont::create(lbl, "chatFont.fnt");
            l->setScale(0.45f);
            l->setColor({200, 200, 225});
            l->setAnchorPoint({1.f, 0.5f});
            l->setPosition({labelX, y});
            m_mainLayer->addChild(l);

            auto inp = TextInput::create(inputW, placeholder);
            inp->setPosition({inputX, y});
            m_mainLayer->addChild(inp);
            return inp;
        };

        m_nameInput         = addRow("Pack Name",     startY,          "My Cool Icons");
        m_authorInput       = addRow("Author",        startY - rowH,   "YourUsername");
        m_gamemodeInput     = addRow("Gamemode",      startY - rowH*2, "cube");
        m_imageUrlInput     = addRow("Image URL",     startY - rowH*3, "https://...");
        m_plistUrlInput     = addRow("Plist URL",     startY - rowH*4, "https://...");
        m_graphicsTypeInput = addRow("Graphics Type", startY - rowH*5, "UHD");

        auto gmHint = CCLabelBMFont::create(
            "cube / ship / ball / ufo / wave / robot / spider / swing / jetpack",
            "chatFont.fnt");
        gmHint->setScale(0.24f);
        gmHint->setColor({120, 120, 155});
        gmHint->setPosition({inputX, startY - rowH * 2.f - 12.f});
        m_mainLayer->addChild(gmHint);

        auto gtHint = CCLabelBMFont::create(
            "UHD / HD / Standard", "chatFont.fnt");
        gtHint->setScale(0.26f);
        gtHint->setColor({120, 120, 155});
        gtHint->setPosition({inputX, startY - rowH * 5.f - 12.f});
        m_mainLayer->addChild(gtHint);

        m_statusLabel = CCLabelBMFont::create("", "chatFont.fnt");
        m_statusLabel->setScale(0.35f);
        m_statusLabel->setColor({220, 80, 80});
        m_statusLabel->setPosition({winSize.width / 2.f, 36.f});
        m_mainLayer->addChild(m_statusLabel);

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        m_mainLayer->addChild(menu);

        auto spr = ButtonSprite::create("Submit");
        spr->setScale(0.8f);
        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(SubmitPackPopup::onSubmit));
        btn->setPosition({winSize.width / 2.f, 16.f});
        menu->addChild(btn);

        return true;
    }

    void setStatus(const char* msg, bool error) {
        if (!m_statusLabel) return;
        m_statusLabel->setString(msg);
        m_statusLabel->setColor(
            error ? ccColor3B{220, 80, 80} : ccColor3B{80, 200, 100});
    }

    void onSubmit(CCObject*) {
        if (m_submitHandle) return;

        std::string name        = m_nameInput         ? m_nameInput->getString()         : "";
        std::string author      = m_authorInput       ? m_authorInput->getString()       : "";
        std::string gamemode    = m_gamemodeInput     ? m_gamemodeInput->getString()     : "";
        std::string imageUrl    = m_imageUrlInput     ? m_imageUrlInput->getString()     : "";
        std::string plistUrl    = m_plistUrlInput     ? m_plistUrlInput->getString()     : "";
        std::string graphicsType = m_graphicsTypeInput ? m_graphicsTypeInput->getString() : "";

        if (name.empty())     { setStatus("Pack name is required.",  true); return; }
        if (gamemode.empty()) { setStatus("Gamemode is required.",   true); return; }

        if (imageUrl.empty()) {
            setStatus("Image URL is required.", true); return;
        }
        if (!isHttpsUrl(imageUrl)) {
            setStatus("Image URL must start with https://.", true); return;
        }
        if (!plistUrl.empty() && !isHttpsUrl(plistUrl)) {
            setStatus("Plist URL must start with https://.", true); return;
        }

        if (containsBanned(name)) {
            setStatus("Pack name contains prohibited words.", true);
            return;
        }
        if (!author.empty() && containsBanned(author)) {
            setStatus("Author name contains prohibited words.", true);
            return;
        }

        int secondsLeft = 0;
        if (onCooldown(secondsLeft)) {
            int minutes = secondsLeft / 60, seconds = secondsLeft % 60;
            std::string msg =
                "Please wait " + std::to_string(minutes) + "m " +
                std::to_string(seconds) + "s before submitting again.";
            setStatus(msg.c_str(), true);
            return;
        }

        auto projectId = Mod::get()->getSettingValue<std::string>("firebase-project-id");
        if (projectId.empty()) {
            setStatus("Firebase Project ID is not configured.", true);
            return;
        }

        auto* acc = GJAccountManager::sharedState();
        if (!acc || acc->m_accountID <= 0) {
            setStatus("You must be logged into Geometry Dash to submit.", true);
            return;
        }
        std::string submittedBy = std::to_string(acc->m_accountID);

        std::string url =
            "https://firestore.googleapis.com/v1/projects/" + projectId +
            "/databases/(default)/documents/pendingIconPacks";
        appendApiKey(url);

        setStatus("Submitting...", false);

        std::string body = buildDoc(name, author, gamemode, imageUrl, plistUrl, graphicsType, submittedBy);
        Ref<SubmitPackPopup> selfRef(this);

        FirebaseAuth::withToken([selfRef, url, body](std::string const& idToken) mutable {
            if (!selfRef) return;

            auto req = web::WebRequest()
                .bodyString(body)
                .header("Content-Type", "application/json");
            if (!idToken.empty())
                req = req.header("Authorization", "Bearer " + idToken);

            selfRef->m_submitHandle = geode::async::spawn(
                req.post(url),
                [selfRef](web::WebResponse response) mutable {
                    if (!selfRef) return;
                    selfRef->m_submitHandle.reset();
                    if (response.ok()) {
                        recordSubmit();
                        selfRef->setStatus(
                            "Submitted! A moderator will review it soon.", false);
                    } else {
                        if (response.code() == 401 || response.code() == 403) {
                            FirebaseAuth::invalidate();
                        }
                        selfRef->setStatus(
                            ("Submit failed (" +
                             std::to_string(response.code()) + ").").c_str(),
                            true);
                    }
                });
        });
    }

public:
    static SubmitPackPopup* create() {
        auto ret = new SubmitPackPopup();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }
};
