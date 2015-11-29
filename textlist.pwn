/*

	About: text list system
	Author:	ziggi

	Instalation:
		Include this file after a_samp.inc

	Usage:
		TextListCreate:example_tl(playerid)
		{
			new items[][TEXTLIST_MAX_ITEM_NAME] = {
				"Test 1",
				"Big Test 2"
			};

			TextList_Open(playerid, TextList:example_tl, items, sizeof(items), "Example header", "Button 1", "Button 2");
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
#pragma library textlist

/*

	Define const

*/

#define TEXTLIST_MAX_ITEMS         30
#define TEXTLIST_MAX_ITEMS_ON_LIST 10
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
	TD_ListItemValue[MAX_PLAYERS][TEXTLIST_MAX_ITEMS][TEXTLIST_MAX_ITEM_NAME],
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
	new call_func[TEXTLIST_MAX_FUNCTION_NAME];
	strcat(call_func, "tlc_");
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

	CancelSelectTextDraw(playerid);
}

stock TextList_Open(playerid, function[], list_items[][], list_count, header[] = "",
                    button1[] = "", button2[] = "", Float:pos_x = 89.0, Float:pos_y = 140.0)
{
	TextList_Close(playerid);

	new items_count = list_count;

	if (items_count > TEXTLIST_MAX_ITEMS) {
		printf("Error: so big list count value (%d, max is %d).", items_count, TEXTLIST_MAX_ITEMS);
		items_count = TEXTLIST_MAX_ITEMS;
	}

	IsOpen[playerid] = true;
	ListCount[playerid] = items_count;
	ListPage[playerid] = 0;
	TD_PosX[playerid] = pos_x;
	TD_PosY[playerid] = pos_y;
	strmid(FunctionName[playerid], function, 0, strlen(function), sizeof(FunctionName[]));
	strmid(ButtonName[playerid][0], button1, 0, strlen(button1), TEXTLIST_MAX_BUTTON_NAME);
	strmid(ButtonName[playerid][1], button2, 0, strlen(button2), TEXTLIST_MAX_BUTTON_NAME);

	for (new i = 0; i < items_count; i++) {
		strmid(TD_ListItemValue[playerid][i], list_items[i], 0, strlen(list_items[i]), TEXTLIST_MAX_ITEM_NAME);
	}

	// header
	if (strlen(header) != 0) {
		TD_HeaderCreate(playerid, header, TD_PosX[playerid], TD_PosY[playerid]);
	}

	TD_SetPage(playerid, ListPage[playerid], ListCount[playerid],
	           TD_PosX[playerid], TD_PosY[playerid],
	           ButtonName[playerid], TD_ListItemValue[playerid]);

	SelectTextDraw(playerid, -5963521);
}

stock TextList_IsOpen(playerid)
{
	return _:IsOpen[playerid];
}

/*

	Private functions

*/

static stock TD_SetPage(playerid, &page_id, items_count, Float:pos_x, Float:pos_y, buttons[][], list_item[][])
{
	// clean old page
	TD_Remove(playerid, TD_ListUp[playerid]);
	TD_Remove(playerid, TD_ListDown[playerid]);
	TD_Remove(playerid, TD_ListPage[playerid]);
	TD_Remove(playerid, TD_ListBox[playerid]);

	TD_Remove(playerid, TD_Button1[playerid]);
	TD_Remove(playerid, TD_Button2[playerid]);

	for (new i = 0; i < TEXTLIST_MAX_ITEMS; i++) {
		TD_Remove(playerid, TD_ListItem[playerid][i]);
	}

	// list
	new pages_count, start_index, end_index;
	GetPaginatorInfo(items_count, page_id, pages_count, start_index, end_index);

	// draw list
	new current_row = 0;

	for (new i = start_index; i < end_index; i++) {
		current_row++;
		TD_ListCreate(playerid, i, current_row, list_item[i], pos_x, pos_y);
	}

	// paginator
	if (pages_count > 1) {
		new string[4];
		format(string, sizeof(string), "%d/%d", page_id + 1, pages_count);

		current_row++;
		TD_PaginatorCreate(playerid, string, pos_x, pos_y + current_row * 20.0);
	}

	// button
	new
		button1_length = strlen(buttons[0]),
		button2_length = strlen(buttons[1]);

	if (button1_length != 0 && button2_length != 0) {
		current_row++;
		TD_ButtonCreate(playerid, TD_Button1[playerid], buttons[0], pos_x - 36.0, pos_y + current_row * 20.0);
		TD_ButtonCreate(playerid, TD_Button2[playerid], buttons[1], pos_x + 36.0, pos_y + current_row * 20.0);
	} else if (button1_length != 0) {
		current_row++;
		TD_ButtonCreate(playerid, TD_Button1[playerid], buttons[0], pos_x, pos_y + current_row * 20.0);
	} else if (button2_length != 0) {
		current_row++;
		TD_ButtonCreate(playerid, TD_Button2[playerid], buttons[1], pos_x, pos_y + current_row * 20.0);
	}
}

static stock GetPaginatorInfo(items_count, &curr_page, &max_page, &start_index, &end_index)
{
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

static stock TD_ListCreate(playerid, item_id, row, text[], Float:pos_x, Float:pos_y)
{
	TD_ListItem[playerid][item_id] = CreatePlayerTextDraw(playerid, pos_x, pos_y + row * 20.0, text);
	PlayerTextDrawLetterSize(playerid, TD_ListItem[playerid][item_id], 0.22, 1.5);
	PlayerTextDrawTextSize(playerid, TD_ListItem[playerid][item_id], 13.0, 135.0);
	PlayerTextDrawAlignment(playerid, TD_ListItem[playerid][item_id], 2);
	PlayerTextDrawColor(playerid, TD_ListItem[playerid][item_id], -1);
	PlayerTextDrawUseBox(playerid, TD_ListItem[playerid][item_id], 1);
	PlayerTextDrawBoxColor(playerid, TD_ListItem[playerid][item_id], 0x212121A0);
	PlayerTextDrawSetShadow(playerid, TD_ListItem[playerid][item_id], 0);
	PlayerTextDrawSetOutline(playerid, TD_ListItem[playerid][item_id], 0);
	PlayerTextDrawBackgroundColor(playerid, TD_ListItem[playerid][item_id], 255);
	PlayerTextDrawFont(playerid, TD_ListItem[playerid][item_id], 1);
	PlayerTextDrawSetProportional(playerid, TD_ListItem[playerid][item_id], 1);
	PlayerTextDrawSetSelectable(playerid, TD_ListItem[playerid][item_id], true);

	PlayerTextDrawShow(playerid, TD_ListItem[playerid][item_id]);
}

static stock TD_PaginatorCreate(playerid, pagestr[], Float:pos_x, Float:pos_y)
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
	PlayerTextDrawColor(playerid, TD_ListDown[playerid], -1);
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
	PlayerTextDrawColor(playerid, TD_ListPage[playerid], -1);
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
	PlayerTextDrawColor(playerid, TD_ListBox[playerid], -1);
	PlayerTextDrawUseBox(playerid, TD_ListBox[playerid], 1);
	PlayerTextDrawBoxColor(playerid, TD_ListBox[playerid], 0x21212160);
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

static stock TD_HeaderCreate(playerid, text[], Float:pos_x, Float:pos_y)
{
	TD_ListHeader[playerid] = CreatePlayerTextDraw(playerid, pos_x, pos_y, text);
	PlayerTextDrawLetterSize(playerid, TD_ListHeader[playerid], 0.3, 1.5);
	PlayerTextDrawTextSize(playerid, TD_ListHeader[playerid], 13.0, 135.0 + 8.0);
	PlayerTextDrawAlignment(playerid, TD_ListHeader[playerid], 2);
	PlayerTextDrawColor(playerid, TD_ListHeader[playerid], -1);
	PlayerTextDrawUseBox(playerid, TD_ListHeader[playerid], 1);
	PlayerTextDrawBoxColor(playerid, TD_ListHeader[playerid], 0xB71C1CAA);
	PlayerTextDrawSetShadow(playerid, TD_ListHeader[playerid], 0);
	PlayerTextDrawSetOutline(playerid, TD_ListHeader[playerid], 0);
	PlayerTextDrawBackgroundColor(playerid, TD_ListHeader[playerid], 255);
	PlayerTextDrawFont(playerid, TD_ListHeader[playerid], 1);
	PlayerTextDrawSetProportional(playerid, TD_ListHeader[playerid], 1);
	
	PlayerTextDrawShow(playerid, TD_ListHeader[playerid]);
}

static stock TD_ButtonCreate(playerid, &PlayerText:button, text[], Float:pos_x, Float:pos_y)
{
	button = CreatePlayerTextDraw(playerid, pos_x, pos_y, text);
	PlayerTextDrawLetterSize(playerid, button, 0.25, 1.4);
	PlayerTextDrawTextSize(playerid, button, 13.0, 68.0);
	PlayerTextDrawAlignment(playerid, button, 2);
	PlayerTextDrawColor(playerid, button, -1);
	PlayerTextDrawUseBox(playerid, button, 1);
	PlayerTextDrawBoxColor(playerid, button, 0x6D4C41AA);
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

public OnPlayerDisconnect(playerid, reason)
{
	if (TextList_IsOpen(playerid)) {
		TextList_Close(playerid);
	}

	CallLocalFunction("TL_OnPlayerDisconnect", "ii", playerid, reason);
	return 1;
}

#if defined _ALS_OnPlayerDisconnect
	#undef OnPlayerDisconnect
#else
	#define _ALS_OnPlayerDisconnect
#endif
#define OnPlayerDisconnect TL_OnPlayerDisconnect
forward OnPlayerDisconnect(playerid, reason);


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
			strmid(value, TD_ListItemValue[playerid][list_id], 0, strlen(TD_ListItemValue[playerid][list_id]), TEXTLIST_MAX_ITEM_NAME);
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
		if (TD_ListUp[playerid] == playertextid) {
			ListPage[playerid]--;
			TD_SetPage(playerid, ListPage[playerid], ListCount[playerid],
			           TD_PosX[playerid], TD_PosY[playerid],
			           ButtonName[playerid], TD_ListItemValue[playerid]);

			response_type = TextListType:TextList_ListUp;
		} else if (TD_ListDown[playerid] == playertextid) {
			ListPage[playerid]++;
			TD_SetPage(playerid, ListPage[playerid], ListCount[playerid],
			           TD_PosX[playerid], TD_PosY[playerid],
			           ButtonName[playerid], TD_ListItemValue[playerid]);

			response_type = TextListType:TextList_ListDown;
		}

		// check on errors
		if (response_type == TextListType:TextList_None) {
			return 0;
		}

		// call function
		new call_func[TEXTLIST_MAX_FUNCTION_NAME];
		strcat(call_func, "tlr_");
		strcat(call_func, FunctionName[playerid]);

		if (funcidx(call_func) != -1) {
			CallLocalFunction(call_func, "iiis", playerid, _:response_type, list_id, value);
		}
	}
	
	CallLocalFunction("TL_OPClickPlayerTextDraw", "ii", playerid, _:playertextid);
	return 1;
}

#if defined _ALS_OPClickPlayerTextDraw
	#undef OPClickPlayerTextDraw
#else
	#define _ALS_OPClickPlayerTextDraw
#endif
#define OPClickPlayerTextDraw TL_OPClickPlayerTextDraw
forward OPClickPlayerTextDraw(playerid, PlayerText:playertextid);
