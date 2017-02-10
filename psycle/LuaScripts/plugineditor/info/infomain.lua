return {
  title = "MAIN MENU",
  keymap = {
    J = "helpmain",
    K = "kmenu",
    Q = "quickmenu",
    S = "@charleft",
    D = "@charright",
    E = "@lineup",
    X = "@linedown",
    A = "@wordleft",
    F = "@wordright",
    O = "@loadpage"
  },
  display = {
    {section = "--Cursor Movement--"},
    {shortdesc = "", desc = "^S char left ^D char right"},
    {shortdesc = "", desc = "^A word left ^F word right"},
    {shortdesc = "", desc = "^E line up   ^X line down"},
    {section = "--Common--"},
    {shortdesc = "^O", desc = "Open File"},
    {section = "--Other Menus--"},
    {shortdesc = "^J", desc = "Help"},
    {shortdesc = "^K", desc = "Block"},
    {shortdesc = "^Q", desc = "Quick"}
  }
}