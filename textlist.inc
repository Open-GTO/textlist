/*

	About: Text List system
	Author:	ziggi

	Installation:
		Include this file after a_samp.inc

	Usage:
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

*/

#if !defined _samp_included
	#error "Please include a_samp or a_npc before textlist"
#endif

#if defined _textlist_included
	#endinput
#endif

#define _textlist_included

/*

	Define const

*/

#if !defined TEXTLIST_MAX_ITEMS
	#define TEXTLIST_MAX_ITEMS 30
#endif

#if !defined TEXTLIST_MAX_ITEMS_ON_LIST
	#define TEXTLIST_MAX_ITEMS_ON_LIST 10
#endif

#define TEXTLIST_MAX_ITEM_NAME     32
#define TEXTLIST_MAX_FUNCTION_NAME 31
#define TEXTLIST_MAX_BUTTON_NAME   12

/*

	Define functions

*/

#define TextListCreate:%0(%1) \
	forward tlc_%0(%1); \
	public tlc_%0(%1)

#define TextListResponse:%0(%1) \
	forward tlr_%0(%1); \
	public tlr_%0(%1)

#define TextList: #

/*

	Enums

*/

enum TextListType {
	TextList_None,
	TextList_Button1,
	TextList_Button2,
	TextList_ListItem,
	TextList_ListUp,
	TextList_ListDown,
	TextList_Cancel,
}

/*

	Vars

*/

static
	bool:IsOpen[MAX_PLAYERS],
	ListCount[MAX_PLAYERS],
	ListPage[MAX_PLAYERS],
	FunctionName[MAX_PLAYERS][TEXTLIST_MAX_FUNCTION_NAME],
	ButtonName[MAX_PLAYERS][2][TEXTLIST_MAX_BUTTON_NAME],
	bool:TD_SkipCancel[MAX_PLAYERS],
	TD_ListItemValue[MAX_PLAYERS][TEXTLIST_MAX_ITEMS][TEXTLIST_MAX_ITEM_NAME],
	TD_ListBgColors[MAX_PLAYERS][TEXTLIST_MAX_ITEMS],
	TD_ListFgColors[MAX_PLAYERS][TEXTLIST_MAX_ITEMS],
	TD_HeaderBgColor[MAX_PLAYERS],
	TD_HeaderFgColor[MAX_PLAYERS],
	TD_PaginatorBgColor[MAX_PLAYERS],
	TD_PaginatorFgColor[MAX_PLAYERS],
	TD_Button1BgColor[MAX_PLAYERS],
	TD_Button1FgColor[MAX_PLAYERS],
	TD_Button2BgColor[MAX_PLAYERS],
	TD_Button2FgColor[MAX_PLAYERS],
	Float:TD_PosX[MAX_PLAYERS],
	Float:TD_PosY[MAX_PLAYERS],
	PlayerText:TD_ListHeader[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
	PlayerText:TD_ListItem[MAX_PLAYERS][TEXTLIST_MAX_ITEMS],
	PlayerText:TD_Button1[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
	PlayerText:TD_Button2[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
	PlayerText:TD_ListUp[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
	PlayerText:TD_ListDown[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
	PlayerText:TD_ListPage[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...},
	PlayerText:TD_ListBox[MAX_PLAYERS] = {PlayerText:INVALID_TEXT_DRAW, ...};

/*

	Public functions

*/

stock TextList_Show(playerid, function[])
{
	new call_func[TEXTLIST_MAX_FUNCTION_NAME] = "tlc_";
	strcat(call_func, function);

	CallLocalFunction(call_func, "i", playerid);
}

stock TextList_Close(playerid)
{
	IsOpen[playerid] = false;
	FunctionName[playerid][0] = '\0';
	ButtonName[playerid][0][0] = '\0';
	ButtonName[playerid][1][0] = '\0';

	TD_Remove(playerid, TD_ListHeader[playerid]);

	TD_Remove(playerid, TD_ListUp[playerid]);
	TD_Remove(playerid, TD_ListDown[playerid]);
	TD_Remove(playerid, TD_ListPage[playerid]);
	TD_Remove(playerid, TD_ListBox[playerid]);

	TD_Remove(playerid, TD_Button1[playerid]);
	TD_Remove(playerid, TD_Button2[playerid]);

	for (new i = 0; i < TEXTLIST_MAX_ITEMS; i++) {
		TD_ListItemValue[playerid][i][0] = '\0';

		TD_Remove(playerid, TD_ListItem[playerid][i]);
	}

	TD_SkipCancel[playerid] = true;
	CancelSelectTextDraw(playerid);
}

stock TextList_Open(playerid, function[], list_items[][], list_size = sizeof(list_items), header[] = "",
                    button1[] = "", button2[] = "", Float:pos_x = 89.0, Float:pos_y = 140.0,
                    select_color = 0xFFA500FF,
                    lists_bg_color[TEXTLIST_MAX_ITEMS] = {0x212121A0, ...},
                    lists_fg_color[TEXTLIST_MAX_ITEMS] = {0xFFFFFFFF, ...},
                    header_bg_color = 0xB71C1CAA, header_fg_color = 0xFFFFFFFF,
                    paginator_bg_color = 0x21212160, paginator_fg_color = 0xFFFFFFFF,
                    button1_bg_color = 0x6D4C41AA, button1_fg_color = 0xFFFFFFFF,
                    button2_bg_color = 0x6D4C41AA, button2_fg_color = 0xFFFFFFFF)
{
	TextList_Close(playerid);

	new items_count = list_size;

	if (items_count > TEXTLIST_MAX_ITEMS) {
		printf("Error (TextList): list count value is too big (%d, max is %d).", items_count, TEXTLIST_MAX_ITEMS);
		items_count = TEXTLIST_MAX_ITEMS;
	}

	IsOpen[playerid] = true;
	ListCount[playerid] = items_count;
	ListPage[playerid] = 0;
	TD_PosX[playerid] = pos_x;
	TD_PosY[playerid] = pos_y;
	TD_ListBgColors[playerid] = lists_bg_color;
	TD_ListFgColors[playerid] = lists_fg_color;
	TD_HeaderBgColor[playerid] = header_bg_color;
	TD_HeaderFgColor[playerid] = header_fg_color;
	TD_PaginatorBgColor[playerid] = paginator_bg_color;
	TD_PaginatorFgColor[playerid] = paginator_fg_color;
	TD_Button1BgColor[playerid] = button1_bg_color;
	TD_Button1FgColor[playerid] = button1_fg_color;
	TD_Button2BgColor[playerid] = button2_bg_color;
	TD_Button2FgColor[playerid] = button2_fg_color;
	TL_strcpy(FunctionName[playerid], function, TEXTLIST_MAX_FUNCTION_NAME);
	TL_strcpy(ButtonName[playerid][0], button1, TEXTLIST_MAX_BUTTON_NAME);
	TL_strcpy(ButtonName[playerid][1], button2, TEXTLIST_MAX_BUTTON_NAME);

	for (new i = 0; i < items_count; i++) {
		TL_strcpy(TD_ListItemValue[playerid][i], list_items[i], TEXTLIST_MAX_ITEM_NAME);
	}

	// header
	if (strlen(header) != 0) {
		TD_HeaderCreate(playerid, header, header_bg_color, header_fg_color, TD_PosX[playerid], TD_PosY[playerid]);
	}

	TD_SetPage(playerid, ListPage[playerid], ListCount[playerid],
	           TD_PosX[playerid], TD_PosY[playerid],
	           ButtonName[playerid], TD_ListItemValue[playerid],
	           lists_bg_color, lists_fg_color,
	           paginator_bg_color, paginator_fg_color,
	           button1_bg_color, button1_fg_color,
	           button2_bg_color, button2_fg_color);

	SelectTextDraw(playerid, select_color);
}

stock TextList_IsOpen(playerid)
{
	return _:IsOpen[playerid];
}

/*

	Private functions

*/

static stock TD_SetPage(playerid, &page_id, items_count,
                        Float:pos_x, Float:pos_y,
                        buttons[][], list_item[][],
                        lists_bg_color[TEXTLIST_MAX_ITEMS], lists_fg_color[TEXTLIST_MAX_ITEMS],
                        paginator_bg_color, paginator_fg_color,
                        button1_bg_color, button1_fg_color,
                        button2_bg_color, button2_fg_color)
{
	// list
	new pages_count, start_index, end_index;
	GetPaginatorInfo(items_count, page_id, pages_count, start_index, end_index);

	new
		bool:IsDefaultItemCount = end_index - start_index == TEXTLIST_MAX_ITEMS_ON_LIST,
		bool:IsListCreated = TD_ListItem[playerid][0] != PlayerText:INVALID_TEXT_DRAW,
		bool:IsPageChanged = page_id != ListPage[playerid];

	if (IsDefaultItemCount && IsListCreated && IsPageChanged) {
		// update list
		new current_row = 0;

		for (new i = start_index; i < end_index; i++) {
			current_row++;
			TD_ListUpdateText(playerid, current_row, list_item[i]);
		}

		// update paginator
		if (pages_count > 1) {
			new string[8];
			format(string, sizeof(string), "%d/%d", page_id + 1, pages_count);

			TD_PaginatorUpdateText(playerid, string);
		}
	} else {
		// remove old page
		TD_Remove(playerid, TD_ListUp[playerid]);
		TD_Remove(playerid, TD_ListDown[playerid]);
		TD_Remove(playerid, TD_ListPage[playerid]);
		TD_Remove(playerid, TD_ListBox[playerid]);

		TD_Remove(playerid, TD_Button1[playerid]);
		TD_Remove(playerid, TD_Button2[playerid]);

		for (new i = 0; i < TEXTLIST_MAX_ITEMS; i++) {
			TD_Remove(playerid, TD_ListItem[playerid][i]);
		}

		// draw list
		new current_row = 0;

		for (new i = start_index; i < end_index; i++) {
			current_row++;
			TD_ListCreate(playerid, i, list_item[i], lists_bg_color[i], lists_fg_color[i],
			              pos_x, pos_y + current_row * 20.0);
		}

		// paginator
		if (pages_count > 1) {
			new string[8];
			format(string, sizeof(string), "%d/%d", page_id + 1, pages_count);

			current_row++;
			TD_PaginatorCreate(playerid, string, paginator_bg_color, paginator_fg_color,
			                   pos_x, pos_y + current_row * 20.0);
		}

		// button
		new
			button1_length = strlen(buttons[0]),
			button2_length = strlen(buttons[1]);

		if (button1_length != 0 && button2_length != 0) {
			current_row++;

			TD_ButtonCreate(playerid, TD_Button1[playerid],
			                buttons[0], button1_bg_color, button1_fg_color,
			                pos_x - 36.0, pos_y + current_row * 20.0);

			TD_ButtonCreate(playerid, TD_Button2[playerid],
			                buttons[1], button2_bg_color, button2_fg_color,
			                pos_x + 36.0, pos_y + current_row * 20.0);
		} else if (button1_length != 0) {
			current_row++;
			TD_ButtonCreate(playerid, TD_Button1[playerid],
			                buttons[0], button1_bg_color, button1_fg_color,
			                pos_x, pos_y + current_row * 20.0);
		} else if (button2_length != 0) {
			current_row++;
			TD_ButtonCreate(playerid, TD_Button2[playerid],
			                buttons[0], button2_bg_color, button2_fg_color,
			                pos_x, pos_y + current_row * 20.0);
		}
	}
}

static stock GetPaginatorInfo(items_count, &curr_page = 0, &max_page = 0, &start_index = 0, &end_index = 0)
{
	if (items_count <= 0) {
		return;
	}

	max_page = items_count / TEXTLIST_MAX_ITEMS_ON_LIST;
	if (items_count % TEXTLIST_MAX_ITEMS_ON_LIST != 0) {
		max_page++;
	}

	if (curr_page < 0) {
		curr_page = 0;
	} else if (curr_page > max_page - 1) {
		curr_page = max_page - 1;
	}

	start_index = curr_page * TEXTLIST_MAX_ITEMS_ON_LIST;
	end_index = start_index + TEXTLIST_MAX_ITEMS_ON_LIST;

	if (items_count % end_index == items_count) {
		end_index = start_index + items_count % TEXTLIST_MAX_ITEMS_ON_LIST;
	}
}

static stock TD_ListCreate(playerid, item_id, text[], bg_color, fg_color, Float:pos_x, Float:pos_y)
{
	TD_ListItem[playerid][item_id] = CreatePlayerTextDraw(playerid, pos_x, pos_y, text);
	PlayerTextDrawLetterSize(playerid, TD_ListItem[playerid][item_id], 0.22, 1.5);
	PlayerTextDrawTextSize(playerid, TD_ListItem[playerid][item_id], 13.0, 135.0);
	PlayerTextDrawAlignment(playerid, TD_ListItem[playerid][item_id], 2);
	PlayerTextDrawColor(playerid, TD_ListItem[playerid][item_id], fg_color);
	PlayerTextDrawUseBox(playerid, TD_ListItem[playerid][item_id], 1);
	PlayerTextDrawBoxColor(playerid, TD_ListItem[playerid][item_id], bg_color);
	PlayerTextDrawSetShadow(playerid, TD_ListItem[playerid][item_id], 0);
	PlayerTextDrawSetOutline(playerid, TD_ListItem[playerid][item_id], 0);
	PlayerTextDrawBackgroundColor(playerid, TD_ListItem[playerid][item_id], 255);
	PlayerTextDrawFont(playerid, TD_ListItem[playerid][item_id], 1);
	PlayerTextDrawSetProportional(playerid, TD_ListItem[playerid][item_id], 1);
	PlayerTextDrawSetSelectable(playerid, TD_ListItem[playerid][item_id], true);

	PlayerTextDrawShow(playerid, TD_ListItem[playerid][item_id]);
}

static stock TD_ListUpdateText(playerid, item_id, text[])
{
	PlayerTextDrawSetString(playerid, TD_ListItem[playerid][item_id], text);
}

static stock TD_PaginatorCreate(playerid, pagestr[], bg_color, fg_color, Float:pos_x, Float:pos_y)
{
	TD_ListUp[playerid] = CreatePlayerTextDraw(playerid, pos_x - 20.0, pos_y, "LD_BEAT:up");
	PlayerTextDrawLetterSize(playerid, TD_ListUp[playerid], 0.0, 0.0);
	PlayerTextDrawTextSize(playerid, TD_ListUp[playerid], 10.0, 13.0);
	PlayerTextDrawAlignment(playerid, TD_ListUp[playerid], 1);
	PlayerTextDrawColor(playerid, TD_ListUp[playerid], -1);
	PlayerTextDrawSetShadow(playerid, TD_ListUp[playerid], 0);
	PlayerTextDrawSetOutline(playerid, TD_ListUp[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, TD_ListUp[playerid], 255);
	PlayerTextDrawFont(playerid, TD_ListUp[playerid], 4);
	PlayerTextDrawSetProportional(playerid, TD_ListUp[playerid], 0);
	PlayerTextDrawSetShadow(playerid, TD_ListUp[playerid], 0);
	PlayerTextDrawSetSelectable(playerid, TD_ListUp[playerid], true);

	TD_ListDown[playerid] = CreatePlayerTextDraw(playerid, pos_x + 20.0 - 10.0, pos_y, "LD_BEAT:down");
	PlayerTextDrawLetterSize(playerid, TD_ListDown[playerid], 0.0, 0.0);
	PlayerTextDrawTextSize(playerid, TD_ListDown[playerid], 10.0, 13.0);
	PlayerTextDrawAlignment(playerid, TD_ListDown[playerid], 1);
	PlayerTextDrawColor(playerid, TD_ListDown[playerid], fg_color);
	PlayerTextDrawSetShadow(playerid, TD_ListDown[playerid], 0);
	PlayerTextDrawSetOutline(playerid, TD_ListDown[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, TD_ListDown[playerid], 255);
	PlayerTextDrawFont(playerid, TD_ListDown[playerid], 4);
	PlayerTextDrawSetProportional(playerid, TD_ListDown[playerid], 0);
	PlayerTextDrawSetShadow(playerid, TD_ListDown[playerid], 0);
	PlayerTextDrawSetSelectable(playerid, TD_ListDown[playerid], true);

	TD_ListPage[playerid] = CreatePlayerTextDraw(playerid, pos_x, pos_y + 1.0, pagestr);
	PlayerTextDrawLetterSize(playerid, TD_ListPage[playerid], 0.2, 1.0);
	PlayerTextDrawAlignment(playerid, TD_ListPage[playerid], 2);
	PlayerTextDrawColor(playerid, TD_ListPage[playerid], fg_color);
	PlayerTextDrawSetShadow(playerid, TD_ListPage[playerid], 0);
	PlayerTextDrawSetOutline(playerid, TD_ListPage[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, TD_ListPage[playerid], 255);
	PlayerTextDrawFont(playerid, TD_ListPage[playerid], 1);
	PlayerTextDrawSetProportional(playerid, TD_ListPage[playerid], 1);
	PlayerTextDrawSetShadow(playerid, TD_ListPage[playerid], 0);

	TD_ListBox[playerid] = CreatePlayerTextDraw(playerid, pos_x, pos_y, "_");
	PlayerTextDrawLetterSize(playerid, TD_ListBox[playerid], 0.0, 1.5);
	PlayerTextDrawTextSize(playerid, TD_ListBox[playerid], 10.0, 135.0);
	PlayerTextDrawAlignment(playerid, TD_ListBox[playerid], 2);
	PlayerTextDrawColor(playerid, TD_ListBox[playerid], fg_color);
	PlayerTextDrawUseBox(playerid, TD_ListBox[playerid], 1);
	PlayerTextDrawBoxColor(playerid, TD_ListBox[playerid], bg_color);
	PlayerTextDrawSetShadow(playerid, TD_ListBox[playerid], 0);
	PlayerTextDrawSetOutline(playerid, TD_ListBox[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, TD_ListBox[playerid], 255);
	PlayerTextDrawFont(playerid, TD_ListBox[playerid], 1);
	PlayerTextDrawSetProportional(playerid, TD_ListBox[playerid], 1);
	PlayerTextDrawSetShadow(playerid, TD_ListBox[playerid], 0);

	PlayerTextDrawShow(playerid, TD_ListUp[playerid]);
	PlayerTextDrawShow(playerid, TD_ListDown[playerid]);
	PlayerTextDrawShow(playerid, TD_ListPage[playerid]);
	PlayerTextDrawShow(playerid, TD_ListBox[playerid]);
}

static stock TD_PaginatorUpdateText(playerid, pagestr[])
{
	PlayerTextDrawSetString(playerid, TD_ListPage[playerid], pagestr);
}

static stock TD_HeaderCreate(playerid, text[], bg_color, fg_color, Float:pos_x, Float:pos_y)
{
	TD_ListHeader[playerid] = CreatePlayerTextDraw(playerid, pos_x, pos_y, text);
	PlayerTextDrawLetterSize(playerid, TD_ListHeader[playerid], 0.3, 1.5);
	PlayerTextDrawTextSize(playerid, TD_ListHeader[playerid], 13.0, 135.0 + 8.0);
	PlayerTextDrawAlignment(playerid, TD_ListHeader[playerid], 2);
	PlayerTextDrawColor(playerid, TD_ListHeader[playerid], fg_color);
	PlayerTextDrawUseBox(playerid, TD_ListHeader[playerid], 1);
	PlayerTextDrawBoxColor(playerid, TD_ListHeader[playerid], bg_color);
	PlayerTextDrawSetShadow(playerid, TD_ListHeader[playerid], 0);
	PlayerTextDrawSetOutline(playerid, TD_ListHeader[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, TD_ListHeader[playerid], 255);
	PlayerTextDrawFont(playerid, TD_ListHeader[playerid], 1);
	PlayerTextDrawSetProportional(playerid, TD_ListHeader[playerid], 1);

	PlayerTextDrawShow(playerid, TD_ListHeader[playerid]);
}

static stock TD_ButtonCreate(playerid, &PlayerText:button, text[], bg_color, fg_color, Float:pos_x, Float:pos_y)
{
	button = CreatePlayerTextDraw(playerid, pos_x, pos_y, text);
	PlayerTextDrawLetterSize(playerid, button, 0.25, 1.4);
	PlayerTextDrawTextSize(playerid, button, 13.0, 68.0);
	PlayerTextDrawAlignment(playerid, button, 2);
	PlayerTextDrawColor(playerid, button, fg_color);
	PlayerTextDrawUseBox(playerid, button, 1);
	PlayerTextDrawBoxColor(playerid, button, bg_color);
	PlayerTextDrawSetShadow(playerid, button, 0);
	PlayerTextDrawSetOutline(playerid, button, 0);
	PlayerTextDrawBackgroundColor(playerid, button, 255);
	PlayerTextDrawFont(playerid, button, 1);
	PlayerTextDrawSetProportional(playerid, button, 1);
	PlayerTextDrawSetSelectable(playerid, button, true);

	PlayerTextDrawShow(playerid, button);
}

static stock TD_Remove(playerid, &PlayerText:td)
{
	PlayerTextDrawHide(playerid, td);
	PlayerTextDrawDestroy(playerid, td);
	td = PlayerText:INVALID_TEXT_DRAW;
}

/*

	OnPlayerDisconnect

*/

public OnPlayerDisconnect(playerid, reason)
{
	if (TextList_IsOpen(playerid)) {
		TextList_Close(playerid);
	}

	#if defined TL_OnPlayerDisconnect
		return TL_OnPlayerDisconnect(playerid, reason);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif

#define OnPlayerDisconnect TL_OnPlayerDisconnect
#if defined TL_OnPlayerDisconnect
	forward TL_OnPlayerDisconnect(playerid, reason);
#endif

/*

	OnPlayerClickPlayerTextDraw

*/

public OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid)
{
	if (TextList_IsOpen(playerid)) {
		new
			TextListType:response_type = TextListType:TextList_None,
			value[TEXTLIST_MAX_ITEM_NAME],
			list_id = -1;

		// check listitem
		for (new i = 0; i < ListCount[playerid]; i++) {
			if (TD_ListItem[playerid][i] == playertextid) {
				list_id = i;
				response_type = TextListType:TextList_ListItem;
			}
		}

		if (list_id != -1) {
			TL_strcpy(value, TD_ListItemValue[playerid][list_id], TEXTLIST_MAX_ITEM_NAME);
		} else {
			value[0] = '\1';
		}

		// check buttons
		if (TD_Button1[playerid] == playertextid) {
			response_type = TextListType:TextList_Button1;
		} else if (TD_Button2[playerid] == playertextid) {
			response_type = TextListType:TextList_Button2;
		}

		// check paginator
		if (TD_ListUp[playerid] == playertextid && ListPage[playerid] != 0) {
			ListPage[playerid]--;
			TD_SetPage(playerid, ListPage[playerid], ListCount[playerid],
			           TD_PosX[playerid], TD_PosY[playerid],
			           ButtonName[playerid], TD_ListItemValue[playerid],
			           TD_ListBgColors[playerid], TD_ListFgColors[playerid],
			           TD_PaginatorBgColor[playerid], TD_PaginatorFgColor[playerid],
			           TD_Button1BgColor[playerid], TD_Button1FgColor[playerid],
			           TD_Button2BgColor[playerid], TD_Button2FgColor[playerid]);

			response_type = TextListType:TextList_ListUp;
		}

		new max_page;
		GetPaginatorInfo(ListCount[playerid], .max_page = max_page);

		if (TD_ListDown[playerid] == playertextid && ListPage[playerid] != max_page - 1) {
			ListPage[playerid]++;
			TD_SetPage(playerid, ListPage[playerid], ListCount[playerid],
			           TD_PosX[playerid], TD_PosY[playerid],
			           ButtonName[playerid], TD_ListItemValue[playerid],
			           TD_ListBgColors[playerid], TD_ListFgColors[playerid],
			           TD_PaginatorBgColor[playerid], TD_PaginatorFgColor[playerid],
			           TD_Button1BgColor[playerid], TD_Button1FgColor[playerid],
			           TD_Button2BgColor[playerid], TD_Button2FgColor[playerid]);

			response_type = TextListType:TextList_ListDown;
		}

		// check on errors
		if (response_type == TextListType:TextList_None) {
			return 0;
		}

		// call function
		new call_func[TEXTLIST_MAX_FUNCTION_NAME] = "tlr_";
		strcat(call_func, FunctionName[playerid]);

		if (funcidx(call_func) != -1) {
			CallLocalFunction(call_func, "iiis", playerid, _:response_type, list_id, value);
			return 1;
		}
	}

	#if defined TL_OnPlayerClickPlayerTextDraw
		return TL_OnPlayerClickPlayerTextDraw(playerid, playertextid);
	#else
		return 0;
	#endif
}
#if defined _ALS_OnPlayerClickPlayerTD
	#undef OnPlayerClickPlayerTextDraw
#else
	#define _ALS_OnPlayerClickPlayerTD
#endif

#define OnPlayerClickPlayerTextDraw TL_OnPlayerClickPlayerTextDraw
#if defined TL_OnPlayerClickPlayerTextDraw
	forward TL_OnPlayerClickPlayerTextDraw(playerid, PlayerText:playertextid);
#endif

/*

	OnPlayerClickTextDraw

*/

public OnPlayerClickTextDraw(playerid, Text:clickedid)
{
	if (clickedid == Text:INVALID_TEXT_DRAW && TD_SkipCancel[playerid]) {
		TD_SkipCancel[playerid] = false;
		return 1;
	}

	if (clickedid == Text:INVALID_TEXT_DRAW && TextList_IsOpen(playerid)) {
		new call_func[TEXTLIST_MAX_FUNCTION_NAME] = "tlr_";
		strcat(call_func, FunctionName[playerid]);

		if (funcidx(call_func) != -1) {
			CallLocalFunction(call_func, "iiis", playerid, _:TextList_Cancel, -1, "cancel");
		}
		return 1;
	}

	#if defined TL_OnPlayerClickTextDraw
		return TL_OnPlayerClickTextDraw(playerid, Text:clickedid);
	#else
		return 0;
	#endif
}
#if defined _ALS_OnPlayerClickTextDraw
	#undef OnPlayerClickTextDraw
#else
	#define _ALS_OnPlayerClickTextDraw
#endif

#define OnPlayerClickTextDraw TL_OnPlayerClickTextDraw
#if defined TL_OnPlayerClickTextDraw
	forward TL_OnPlayerClickTextDraw(playerid, Text:clickedid);
#endif

/*

	Helpers

*/

static stock TL_strcpy(result[], source[], const size = sizeof(result))
{
	result[0] = 0;
	return strcat(result, source, size);
}
