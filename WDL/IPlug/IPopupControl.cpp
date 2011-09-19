#include "IPopupControl.h"

/*
Simplified usage description:

Sample xxx.h:
class XXX : public IPlug
{
public:

	XXX(IPlugInstanceInfo instanceInfo);
	~XXX() {}

private:
	IPopupControl* filePopup;
};

Sample xxx.cpp:
	filePopup->AddItem("&Save", (void*)1);
	filePopup->AddItem("Save &As...", (void*)2);
	filePopup->AddItem("&Delete", (void*)3);
	filePopup->AddItem("<SEP>");
	filePopup->AddItem("Se&ttings", (void*)4);
	filePopup->AddItem("<SEP>");
	filePopup->AddItem("<SUB>Sub menu");
	filePopup->AddItem("Sub item", (void*)5);
	filePopup->AddItem("<SUB>Sub sub menu", (void*)6);
	filePopup->AddItem("Sub sub item 1", (void*)7);
	filePopup->AddItem("Sub sub item 2", (void*)8);
	filePopup->AddItem("</SUB>");
	filePopup->AddItem("</SUB>");

	
*/

//////////////////////////////////////////////////////////////////////////
// IPopupControl
//////////////////////////////////////////////////////////////////////////

void IPopupControl::GenSubMenu(HMENU menu, int *x, WDL_PtrList<char> *items, int curitem)
{
	int pos=0;
	while (*x < items->GetSize())
	{
		MENUITEMINFO mi={sizeof(mi), MIIM_ID|MIIM_STATE|MIIM_TYPE, MFT_STRING, 0, m_startitem + *x, NULL, NULL, NULL, 0};
		mi.dwTypeData = (char *)items->Get(*x);
		//mi.fState = curitem == *x ?MFS_CHECKED:0;

		(*x) ++; // advance to next item

		if (!strcmp(mi.dwTypeData,"<SEP>")) mi.fType=MFT_SEPARATOR;
		else if (!strcmp(mi.dwTypeData,"</SUB>")) break; // done!
		else if (!strncmp(mi.dwTypeData,"<SUB>",5))
		{
			mi.hSubMenu= CreatePopupMenu();
			GenSubMenu(mi.hSubMenu,x,items,curitem);
			mi.fMask |= MIIM_SUBMENU;
			mi.dwTypeData += 5; // skip <SUB>
		}
		InsertMenuItem(menu,pos++,TRUE,&mi);
	}
}

void IPopupControl::OnMouseDown(int x, int y, IMouseMod* pMod)
{
	if (IsGrayed() || IsHidden() || ignoreMouse(pMod)) {
		return;
	}

	mValue = 1.0;
	SetDirty();

	// * get the current cursor position in screen coordinates
	GetCursorPos(&m_menuposition); 

	// * make sure the menu is aligned to the UI
	m_menuposition.x -= x;
	m_menuposition.x += mRECT.L;
	m_menuposition.y -= y;
	m_menuposition.y += mRECT.T;
}

void IPopupControl::OnMouseUp(int x, int y, IMouseMod* pMod)
{
	if (IsGrayed() || IsHidden() || ignoreMouse(pMod)) {
		return;
	}

	mValue = 0.0;
	SetDirty();

	// * if the mouse is still on the button show a popup menu... 
	POINT p;
	GetCursorPos(&p); 
	p.x -= x;
	p.x += mRECT.L;
	p.y -= y;
	p.y += mRECT.T;

	if (((m_menuposition.x > p.x) || (m_menuposition.x + mBitmap.W < p.x)) || ((m_menuposition.y > p.y) || (m_menuposition.y + (mBitmap.H / mBitmap.N) < p.y)))
		return;

	// * create the menu based on the user setup
	int menu_id=0;
	HMENU m_menu=CreatePopupMenu();
	GenSubMenu(m_menu,&menu_id,&m_items,m_curitem);

	// * show the menu
	HWND h = (HWND)mPlug->GetGUI()->GetWindow();
	int ret=TrackPopupMenu(m_menu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_NONOTIFY,m_menuposition.x,m_menuposition.y + (mBitmap.H / mBitmap.N),0,h,NULL);

	// * destroy the menu
	DestroyMenu(m_menu);

	// * remember the selection
	m_curitem = ret;

	// * track menu and notify the plugin
	SetDirty();
	mPlug->OnParamChange((long)m_itemdatas.Get(m_curitem));
}

