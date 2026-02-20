# 🎨 Community Icons (Geode Mod)

An open-source [Geode SDK](https://geode-sdk.org) mod for **Geometry Dash 2.2** that allows players to expand their collection with community-made icons using the [More Icons](https://geode-sdk.orgmods/hiimjustin000.more_icons) framework.

---

## ✨ Features
- **Global Browser**: View custom icons uploaded by the community in-game.
- **Dynamic Injection**: Uses the **More Icons API** to add new slots without replacing vanilla icons.
- **Auto-Sync**: Downloaded icons appear immediately in your Icon Kit.
- **Creator Credit**: Every icon in the workshop displays the original artist's name.

## 🛠️ Technical Stack
- **Mod Framework:** [Geode SDK](https://geode-sdk.org)
- **Dependency:** [More Icons](https://geode-sdk.orgmods/hiimjustin000.more_icons) by hiimjustin000
- **Backend:** Node.js + Express (Hosted on [Render](https://render.com))
- **Database:** MongoDB Atlas

## 🚀 How it Works (For Users)
1. Install **More Icons** from the Geode Index.
2. Open the **Community Icons** tab in the Icon Kit.
3. Click "Download" on an icon you like.
4. The icon is saved to your local storage and registered as a new slot in the More Icons menu!

## 💻 Developer Setup
To build this mod, you must have the **More Icons** headers linked in your environment.

```bash
git clone https://github.com/Bluubr/community-icons
cd community-icons
geode build
