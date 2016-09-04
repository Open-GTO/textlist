# TextList
Text Draw list system.

# Screens
Working vehicle tuning system from Open-GTO gamemode. You can see sources [here](https://github.com/Open-GTO/Open-GTO/blob/master/sources/services/tuning.pwn).
![tunning textlist](http://i.imgur.com/x39yVSK.png)
![color textlist](http://i.imgur.com/XI7vYI6.png)

# Functions
#### Show created TextList
```Pawn
TextList_Show(playerid, function[])
```

#### Open TextList
```Pawn
TextList_Open(playerid, function[], list_items[][], list_size = sizeof(list_items), header[] = "",
              button1[] = "", button2[] = "", Float:pos_x = 89.0, Float:pos_y = 140.0,
              select_color = 0xFFA500FF,
              lists_bg_color[TEXTLIST_MAX_ITEMS] = {0x212121A0, ...},
              lists_fg_color[TEXTLIST_MAX_ITEMS] = {0xFFFFFFFF, ...},
              header_bg_color = 0xB71C1CAA, header_fg_color = 0xFFFFFFFF,
              paginator_bg_color = 0x21212160, paginator_fg_color = 0xFFFFFFFF,
              button1_bg_color = 0x6D4C41AA, button1_fg_color = 0xFFFFFFFF,
              button2_bg_color = 0x6D4C41AA, button2_fg_color = 0xFFFFFFFF)
```

#### Close TextList
```Pawn
TextList_Close(playerid);
```

#### Is TextList opened
```Pawn
TextList_IsOpen(playerid);
```

# Callbacks
Each TextList has its own handler function, it looks as follows:
```Pawn
TextListResponse:example_tl(playerid, TextListType:response, itemid, itemvalue[])
{
    return 1;
}
```
This function is called when a user interacts with TextList.

**TextListType** can have these values:
- TextList_None
- TextList_Button1
- TextList_Button2
- TextList_ListItem
- TextList_ListUp
- TextList_ListDown
- TextList_Cancel

# Defines
Directive | Default value | Can be redefined
----------|---------------|------------
TEXTLIST_MAX_ITEMS | 30 | yes
TEXTLIST_MAX_ITEMS_ON_LIST | 10 | yes
TEXTLIST_MAX_ITEM_NAME | 32 | no
TEXTLIST_MAX_FUNCTION_NAME | 31 | no
TEXTLIST_MAX_BUTTON_NAME | 12 | no

# Usage
The system provides the ability to create a function to open TextList, this is useful when multiple calls one list (mostly used when creating nested menus):
```Pawn
TextListCreate:example_tl(playerid)
{
	new items[][TEXTLIST_MAX_ITEM_NAME] = {
		"Test 1",
		"Big Test 2"
	};

	new bg_colors[TEXTLIST_MAX_ITEMS] = {
		0xFF0000FF,
		0x00FF00FF
	};

	TextList_Open(playerid, TextList:example_tl, items, sizeof(items),
	              "Example header",
	              "Button 1", "Button 2",
	              .lists_bg_color = bg_colors);
}

TextListResponse:example_tl(playerid, TextListType:response, itemid, itemvalue[])
{
	new string[128];
	format(string, sizeof(string), " %d | %d | %d | %s", playerid, _:response, itemid, itemvalue);
	SendClientMessage(playerid, -1, string);
	return 1;
}
```
And you should use TextList_Show for opening created TextList:
```Pawn
TextList_Show(playerid, TextList:example_tl);
```
Of course you can not use the system, you can do everything without TextListCreate.
