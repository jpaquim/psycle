///\file
///\brief pointer handler for psycle::host::CChildView, private header
#pragma once
#include <psycle/host/detail/project.hpp>
#include "Psycle.hpp"

namespace psycle { namespace host {

void CChildView::OnRButtonDown( UINT nFlags, CPoint point) {	
	//Right mouse button behaviour (OnRButtonDown() and OnRButtonUp()) extended by sampler.
	SetCapture();			
	if(viewMode == view_modes::machine) { // User is in machine view mode
		Song::scoped_lock lock(projects_->active_project()->song());
		canvas::Event ev;
		ev.type = canvas::Event::BUTTON_PRESS;
		ev.x = point.x;
		ev.y = point.y;
		ev.button = 3;
		ev.shift = nFlags;
		machine_view_->OnEvent(&ev);
		return;
	}
	CWnd::OnRButtonDown(nFlags,point);
}

void CChildView::OnRButtonUp(UINT nFlags, CPoint point) {
	ReleaseCapture();
	if (viewMode == view_modes::machine) {
		canvas::Event ev;
		ev.type = canvas::Event::BUTTON_RELEASE;
		ev.x = point.x;
		ev.y = point.y;
		ev.button = 3;
		ev.shift = nFlags;
		machine_view_->OnEvent(&ev);
		return;
	}
	pattern_view()->Repaint(PatternView::draw_modes::all);
	CWnd::OnRButtonUp(nFlags,point);
}

void CChildView::OnContextMenu(CWnd* pWnd, CPoint point) {			
	if (viewMode == view_modes::pattern) {
		pattern_view_->OnContextMenu(pWnd, point);
	}
	CWnd::OnContextMenu(pWnd,point);
}

void CChildView::OnLButtonDown( UINT nFlags, CPoint point) {
	SetCapture();
	if( viewMode == view_modes::machine) {
		Song::scoped_lock lock(projects_->active_project()->song());
		canvas::Event ev;
		ev.type = canvas::Event::BUTTON_PRESS;
		ev.x = point.x;
		ev.y = point.y;
		ev.button = 1;
		ev.shift = nFlags;
		machine_view_->OnEvent(&ev);
		return;
	} else
	if (viewMode==view_modes::pattern) {			
		pattern_view_->OnLButtonDown(nFlags, point);
	}//<-- End LBUTTONPRESING/VIEWMODE if statement
	CWnd::OnLButtonDown(nFlags,point);
}

void CChildView::OnLButtonUp(UINT nFlags, CPoint point) {
	ReleaseCapture();			
	if (viewMode == view_modes::machine) {
		canvas::Event ev;
		ev.type = canvas::Event::BUTTON_RELEASE;
		ev.x = point.x;
		ev.y = point.y;
		ev.button = 1;
		ev.shift = nFlags;
		machine_view_->OnEvent(&ev);
		return;
	} else 
	if (viewMode == view_modes::pattern) {
		pattern_view_->OnLButtonUp(nFlags, point);
	}
	CWnd::OnLButtonUp(nFlags,point);
}

void CChildView::OnMouseMove( UINT nFlags, CPoint point ) {
	if (viewMode == view_modes::machine) {
		canvas::Event ev;
		ev.type = canvas::Event::MOTION_NOTIFY;
		ev.x = point.x;
		ev.y = point.y;
		if ( nFlags & MK_LBUTTON )
			ev.button = 1;
		else if ( nFlags & MK_RBUTTON )
			ev.button = 3;
		else
			ev.button = 0;
		ev.shift = nFlags;
		machine_view_->OnEvent(&ev);
		return;
	}  else 
	if (viewMode == view_modes::pattern) {
		pattern_view_->OnMouseMove(nFlags, point);
	}
	CWnd::OnMouseMove(nFlags,point);
}

void CChildView::OnLButtonDblClk( UINT nFlags, CPoint point ) {	
	switch (viewMode) {
		case view_modes::machine: // User is in machine view mode
			canvas::Event ev;
			ev.type = canvas::Event::BUTTON_2PRESS;
			ev.x = point.x;
			ev.y = point.y;
			ev.button = 0;
			ev.shift = nFlags;
			machine_view_->OnEvent(&ev);				
		break;				
		case view_modes::pattern: // User is in pattern view mode
			if (( point.y >= pattern_view()->YOFFSET ) && (point.x >= pattern_view()->XOFFSET))
			{
				const int ttm = pattern_view()->tOff + (point.x-pattern_view()->XOFFSET)/pattern_view()->ROWWIDTH;
				const int nl = pattern_view()->pattern()->beats() * projects_->active_project()->beat_zoom();

				pattern_view()->StartBlock(ttm,0,0);
				pattern_view()->EndBlock(ttm,nl-1,8);
				pattern_view()->blockStart = false;
			}
			break;
	}
	CWnd::OnLButtonDblClk(nFlags,point);
}

BOOL CChildView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) {
	if (viewMode == view_modes::pattern) {
		pattern_view_->OnMouseWheel(nFlags, zDelta, pt);
	}
	return CWnd ::OnMouseWheel(nFlags, zDelta, pt);
}

void CChildView::OnMButtonDown(UINT nFlags, CPoint point) {
	pattern_view_->OnMButtonDown(nFlags, point);
}

void CChildView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar){
	if (viewMode == view_modes::pattern) {
		pattern_view_->OnVScroll(nSBCode, nPos, pScrollBar);
	}
	CWnd ::OnVScroll(nSBCode, nPos, pScrollBar);
}


void CChildView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) {
	if (viewMode == view_modes::pattern) {
		pattern_view_->OnHScroll(nSBCode, nPos, pScrollBar);
	}
	CWnd ::OnHScroll(nSBCode, nPos, pScrollBar);
}

}}
