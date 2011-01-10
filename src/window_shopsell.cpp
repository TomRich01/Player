/////////////////////////////////////////////////////////////////////////////
// This file is part of EasyRPG Player.
// 
// EasyRPG Player is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// EasyRPG Player is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with EasyRPG Player. If not, see <http://www.gnu.org/licenses/>.
/////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <iomanip>
#include <sstream>
#include "window_shopsell.h"
#include "game_party.h"

////////////////////////////////////////////////////////////
Window_ShopSell::Window_ShopSell(int ix, int iy, int iwidth, int iheight) :
	Window_Item(ix, iy, iwidth, iheight),
	party_window(NULL) {}

////////////////////////////////////////////////////////////
Window_ShopSell::~Window_ShopSell() {
}

////////////////////////////////////////////////////////////
bool Window_ShopSell::CheckEnable(int item_id) {
	return true;
}

void Window_ShopSell::Update(void) {
	Window_Selectable::Update();
	if (active) {
		if (party_window)
			party_window->SetItem(GetItemId());
	}
}

void Window_ShopSell::SetPartyWindow(Window_Party *w) {
	party_window = w;
}
