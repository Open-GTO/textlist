# textlist
Text List system

# Functions
```Pawn
// show created TextList
TextList_Show(playerid, function[]);
// open TextList
TextList_Open(playerid, function[], list_items[][], list_size = sizeof(list_items), header[] = "",
              button1[] = "", button2[] = "", Float:pos_x = 89.0, Float:pos_y = 140.0,
              select_color = 0xFFA500FF,
              lists_bg_color[TEXTLIST_MAX_ITEMS] = {0x212121A0, ...},
              lists_fg_color[TEXTLIST_MAX_ITEMS] = {0xFFFFFFFF, ...},
              header_bg_color = 0xB71C1CAA, header_fg_color = 0xFFFFFFFF,
              paginator_bg_color = 0x21212160, paginator_fg_color = 0xFFFFFFFF,
              button1_bg_color = 0x6D4C41AA, button1_fg_color = 0xFFFFFFFF,
              button2_bg_color = 0x6D4C41AA, button2_fg_color = 0xFFFFFFFF);
// close TextList
TextList_Close(playerid);
// is TextList opened
TextList_IsOpen(playerid);
```

# Usage
You can use `TextListCreate:` and `TextListResponse:` prefixes:
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
