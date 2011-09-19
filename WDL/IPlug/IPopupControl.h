#ifndef __IPOPUPCONTROL__
#define __IPOPUPCONTROL__

#include "IControl.h"
#include "lice.h"
#include "../wingui/virtwnd.h"

#ifdef __APPLE__
#include "swell.h"
#endif

// A switch that shows a popup menu
class IPopupControl : public ISwitchControl
{
public:

	IPopupControl(IPlugBase* pPlug, int x, int y, int paramIdx, IBitmap* pBitmap, int startitem)
		:	ISwitchControl(pPlug, x, y, paramIdx, pBitmap), m_startitem(startitem), m_useRightMouse(false)  {
	}
	~IPopupControl() {
	}

	// * my own strdup since I didn't find a version for OSX
	static char* aastrdup (const char *s) {
		if (s == NULL) return NULL;
		char *d = (char *)(malloc (strlen (s) + 1));
		if (d == NULL) return NULL;
		strcpy (d,s);
		return d;
	}

	// * simple item management
	int GetCurSel() { if (m_items.Get(m_curitem)) return m_curitem; return -1; }
	void SetCurSel(int sel) { if (!m_items.Get(sel)) sel=-1; if (m_curitem != sel) { m_curitem=sel; Redraw(); } }
	int GetCount() { return m_items.GetSize(); }
	void Empty() { m_items.Empty(true, free); m_itemdatas.Empty(); }
    
    bool GetUseRightMouse() { return m_useRightMouse; }
    void SetUseRightMouse(bool bUse) { m_useRightMouse = bUse; }

	// * simple item editor
	int AddItem(const char *str, void *data=NULL, const char *file=NULL) { m_items.Add(aastrdup(str)); m_itemdatas.Add(data); return m_items.GetSize()-1; }
	bool RemoveLastItem() {int idx = m_items.GetSize()-1; m_items.Delete(idx); m_itemdatas.Delete(idx); };
	
	// * some helpers
	int GetSize() { return m_items.GetSize(); }
	const char *GetItem(int item) { return m_items.Get(item); }
	void *GetItemData(int item) { return m_itemdatas.Get(item); }

	// * UI interaction
	void OnMouseDown(int x, int y, IMouseMod* pMod);
	void OnMouseUp(int x, int y, IMouseMod* pMod);

	// * create the menu
	void GenSubMenu(HMENU menu, int *x, WDL_PtrList<char> *items, int curitem);
    
    

private:
    
    bool ignoreMouse(IMouseMod* pMod) { return m_useRightMouse ? pMod->L : pMod->R; }
    
	int m_align;
	int m_startitem;
	int m_curitem;
	POINT m_menuposition;
	HMENU m_menu;
    
    bool m_useRightMouse;

	WDL_PtrList<char> m_items;
	WDL_PtrList<void> m_itemdatas;
};
#endif /* __IPOPUPCONTROL__ */