// SetPathDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "SetPathDlg.h"
#include "afxdialogex.h"
#pragma warning (disable : 4018)


// CSetPathDlg �Ի���

IMPLEMENT_DYNAMIC(CSetPathDlg, CDialog)

CSetPathDlg::CSetPathDlg(deque<PathInfo>& recent_path, wstring current_path, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SET_PATH_DIALOG, pParent), m_recent_path(recent_path), m_current_path(current_path)
{

}

CSetPathDlg::~CSetPathDlg()
{
}

void CSetPathDlg::QuickSearch(const wstring & key_word)
{
	m_search_result.clear();
	for (size_t i{}; i < m_recent_path.size(); i++)
	{
		if (CCommon::StringFindNoCase(m_recent_path[i].path, key_word) != wstring::npos)
			m_search_result.push_back(i);
	}
}

void CSetPathDlg::ShowPathList()
{
	m_path_list.EnableWindow(TRUE);
	if (!m_searched)		//��ʾ������Ŀ
	{
		m_path_list.DeleteAllItems();
		CString path_str;
		for (int i{}; i < m_recent_path.size(); i++)
		{
			CString str;
			str.Format(_T("%d"), i + 1);
			m_path_list.InsertItem(i, str);

			SetListRowData(i, m_recent_path[i]);
		}
	}
	else		//ֻ��ʾ�����������Ŀ
	{
		if (m_search_result.empty())
		{
			m_path_list.DeleteAllItems();
			m_path_list.InsertItem(0, _T(""));
			m_path_list.SetItemText(0, 1, CCommon::LoadText(IDS_NO_RESULT_TO_SHOW));
			m_path_list.EnableWindow(FALSE);
			return;
		}

		int item_num_before = m_path_list.GetItemCount();
		int item_num_after = m_search_result.size();
		//�����ǰ�б�����Ŀ������С��ԭ���ģ���ֱ�����ԭ���б������е���Ŀ���������
		if (item_num_after < item_num_before)
		{
			m_path_list.DeleteAllItems();
			item_num_before = 0;
		}
		CString str;
		for (int i{}; i < item_num_after; i++)
		{
			str.Format(_T("%u"), m_search_result[i] + 1);
			if (i >= item_num_before)	//�����ǰ�б��е���Ŀ��������֮ǰ������������Ҫ�ڲ���ʱ�����µ���Ŀ
			{
				m_path_list.InsertItem(i, str);
			}
			m_path_list.SetItemText(i, 0, str);
			SetListRowData(i, m_recent_path[m_search_result[i]]);
		}
	}
}

void CSetPathDlg::SetButtonsEnable(bool enable)
{
	GetDlgItem(IDOK)->EnableWindow(enable);
	//GetDlgItem(IDC_DELETE_PATH_BUTTON)->EnableWindow(enable);
}

void CSetPathDlg::CalculateColumeWidth(vector<int>& width)
{
	CRect rect;
	m_path_list.GetWindowRect(rect);
	width.resize(6);

	width[3] = width[4] = rect.Width() / 10;
	width[5] = rect.Width() / 7;

	width[0] = theApp.DPI(40);
	width[1] = width[2] = (rect.Width() - width[3] - width[4] - width[5] - width[0] - theApp.DPI(20)) / 2;
}

void CSetPathDlg::SetListRowData(int index, const PathInfo & path_info)
{
	CFilePathHelper path_helper(path_info.path);
	m_path_list.SetItemText(index, 1, path_helper.GetFolderName().c_str());
	m_path_list.SetItemText(index, 2, path_info.path.c_str());

	CString str;
	str.Format(_T("%d"), path_info.track + 1);
	m_path_list.SetItemText(index, 3, str);

	str.Format(_T("%d"), path_info.track_num);
	m_path_list.SetItemText(index, 4, str);

	Time total_time{ path_info.total_time };
	m_path_list.SetItemText(index, 5, total_time.time2str3().c_str());
}

void CSetPathDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PATH_EDIT, m_path_name);
	//DDX_Control(pDX, IDC_LIST1, m_path_list);
	DDX_Control(pDX, IDC_PATH_LIST, m_path_list);
	DDX_Control(pDX, IDC_SEARCH_EDIT, m_search_edit);
}

wstring CSetPathDlg::GetSelPath() const
{
	if (SelectValid())
		return m_recent_path[m_path_selected].path;
	else
		return wstring();
}

int CSetPathDlg::GetTrack() const
{
	if (SelectValid())
		return m_recent_path[m_path_selected].track;
	else
		return 0;
}

int CSetPathDlg::GetPosition() const
{
	if (SelectValid())
		return m_recent_path[m_path_selected].position;
	else
		return 0;
}

SortMode CSetPathDlg::GetSortMode() const
{
	if (SelectValid())
		return m_recent_path[m_path_selected].sort_mode;
	else
		return SM_FILE;
}

bool CSetPathDlg::SelectValid() const
{
	return (m_path_selected >= 0 && m_path_selected < m_recent_path.size());
}


BEGIN_MESSAGE_MAP(CSetPathDlg, CDialog)
//	ON_LBN_SELCHANGE(IDC_LIST1, &CSetPathDlg::OnLbnSelchangeList1)
	ON_WM_DESTROY()
	//ON_BN_CLICKED(IDC_DELETE_PATH_BUTTON, &CSetPathDlg::OnBnClickedDeletePathButton)
	ON_NOTIFY(NM_CLICK, IDC_PATH_LIST, &CSetPathDlg::OnNMClickPathList)
	ON_NOTIFY(NM_RCLICK, IDC_PATH_LIST, &CSetPathDlg::OnNMRClickPathList)
	ON_NOTIFY(NM_DBLCLK, IDC_PATH_LIST, &CSetPathDlg::OnNMDblclkPathList)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_OPEN_FOLDER, &CSetPathDlg::OnBnClickedOpenFolder)
	ON_COMMAND(ID_PLAY_PATH, &CSetPathDlg::OnPlayPath)
	ON_COMMAND(ID_DELETE_PATH, &CSetPathDlg::OnDeletePath)
	ON_COMMAND(ID_BROWSE_PATH, &CSetPathDlg::OnBrowsePath)
	ON_COMMAND(ID_CLEAR_INVALID_PATH, &CSetPathDlg::OnClearInvalidPath)
	ON_WM_INITMENU()
	ON_EN_CHANGE(IDC_SEARCH_EDIT, &CSetPathDlg::OnEnChangeSearchEdit)
	ON_BN_CLICKED(IDC_CLEAR_BUTTON, &CSetPathDlg::OnBnClickedClearButton)
END_MESSAGE_MAP()


// CSetPathDlg ��Ϣ�������


BOOL CSetPathDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	CenterWindow();

	m_path_name.SetWindowText(m_current_path.c_str());

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);		// ����Сͼ��

	//�����б�ؼ�������ɫ
	//m_path_list.SetColor(theApp.m_app_setting_data.theme_color);

	//��ʼ�������б�ؼ�
	vector<int> width;
	CalculateColumeWidth(width);
	m_path_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
	m_path_list.InsertColumn(0, CCommon::LoadText(IDS_NUMBER), LVCFMT_LEFT, width[0]);
	m_path_list.InsertColumn(1, CCommon::LoadText(IDS_FOLDER), LVCFMT_LEFT, width[1]);
	m_path_list.InsertColumn(2, CCommon::LoadText(IDS_PATH), LVCFMT_LEFT, width[2]);
	m_path_list.InsertColumn(3, CCommon::LoadText(IDS_TRACK_PLAYED), LVCFMT_LEFT, width[3]);
	m_path_list.InsertColumn(4, CCommon::LoadText(IDS_TRACK_TOTAL_NUM), LVCFMT_LEFT, width[4]);
	m_path_list.InsertColumn(5, CCommon::LoadText(IDS_TOTAL_LENGTH), LVCFMT_LEFT, width[5]);

	ShowPathList();
	m_search_edit.SetFocus();		//��ʼʱ���������õ�������

	SetButtonsEnable(false);

	m_search_edit.SetCueBanner(CCommon::LoadText(IDS_SEARCH_HERE), TRUE);

	//��ȡ��ʼʱ���ڵĴ�С
	CRect rect;
	GetWindowRect(rect);
	m_min_size.cx = rect.Width();
	m_min_size.cy = rect.Height();

	//��ʼ����ʾ��Ϣ
	m_Mytip.Create(this, TTS_ALWAYSTIP);
	m_Mytip.AddTool(GetDlgItem(IDC_CLEAR_BUTTON), CCommon::LoadText(IDS_CLEAR_SEARCH_RESULT));
	m_Mytip.AddTool(&m_search_edit, CCommon::LoadText(IDS_INPUT_KEY_WORD));

	//�����б�ؼ�����ʾ�����ö������ڽ����������˴��ڵĸ����ھ����ö�����ʱ����ʾ��Ϣ�ڴ������������
	m_path_list.GetToolTips()->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	m_Mytip.SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	//��ʼ���Ҽ��˵�
	m_menu.LoadMenu(IDR_SET_PATH_POPUP_MENU);
	m_menu.GetSubMenu(0)->SetDefaultItem(ID_PLAY_PATH);

	return FALSE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


//void CSetPathDlg::OnLbnSelchangeList1()
//{
//	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	m_path_selected = m_path_list.GetCurSel();
//}


void CSetPathDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
	//m_path_selected = m_path_list.GetCurSel();
}


//void CSetPathDlg::OnBnClickedDeletePathButton()
//{
//	// TODO: �ڴ���ӿؼ�֪ͨ����������
//	//m_path_selected = m_path_list.GetCurSel();
//	if (SelectValid())
//	{
//		m_recent_path.erase(m_recent_path.begin() + m_path_selected);	//ɾ��ѡ�е�·��
//		ShowPathList();		//������ʾ·���б�
//	}
//}


//BOOL CSetPathDlg::OnCommand(WPARAM wParam, LPARAM lParam)
//{
//	// TODO: �ڴ����ר�ô����/����û���
//	if (wParam == ID_FILE_OPEN_FOLDER)
//	{
//		OnCancel();		//����ˡ�����·������ť��ر�����·���Ի���
//	}
//
//	return CDialog::OnCommand(wParam, lParam);
//}


void CSetPathDlg::OnNMClickPathList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (!m_searched)	//��������б�������״̬����ǰѡ������кž�����Ŀ������
	{
		m_path_selected = pNMItemActivate->iItem;
	}
	else
	{
		CString str;
		str = m_path_list.GetItemText(pNMItemActivate->iItem, 0);
		m_path_selected = _ttoi(str) - 1;
	}

	bool enable = (m_path_selected > 0);
	SetButtonsEnable(enable);
	*pResult = 0;
}


void CSetPathDlg::OnNMRClickPathList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_path_selected = pNMItemActivate->iItem;
	SetButtonsEnable(m_path_selected != -1);

	//�����Ҽ��˵�
	CMenu* pContextMenu = m_menu.GetSubMenu(0);	//��ȡ��һ�������˵�
	CPoint point1;	//����һ������ȷ�����λ�õ�λ��  
	GetCursorPos(&point1);	//��ȡ��ǰ����λ�ã��Ա�ʹ�ò˵����Ը�����
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, this); //��ָ��λ����ʾ�����˵�

	*pResult = 0;
}


void CSetPathDlg::OnNMDblclkPathList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (!m_searched)	//��������б�������״̬����ǰѡ������кž�����Ŀ������
	{
		m_path_selected = pNMItemActivate->iItem;
	}
	else
	{
		CString str;
		str = m_path_list.GetItemText(pNMItemActivate->iItem, 0);
		m_path_selected = _ttoi(str) - 1;
	}

	bool enable = (m_path_selected > 0);
	SetButtonsEnable(enable);
	if(enable)
		OnOK();
	*pResult = 0;
}


void CSetPathDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//���ƴ�����С��С
	lpMMI->ptMinTrackSize.x = m_min_size.cx;		//������С���
	lpMMI->ptMinTrackSize.y = m_min_size.cy;		//������С�߶�

	CDialog::OnGetMinMaxInfo(lpMMI);
}


void CSetPathDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if (nType != SIZE_MINIMIZED && m_path_list.m_hWnd)
	{
		int list_width{ cx - theApp.DPI(29) };
		vector<int> width;
		CalculateColumeWidth(width);
		for (size_t i{}; i < width.size(); i++)
			m_path_list.SetColumnWidth(i, width[i]);
	}
}


void CSetPathDlg::OnCancel()
{
	// TODO: �ڴ����ר�ô����/����û���
	DestroyWindow();

	//CDialog::OnCancel();
}


void CSetPathDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���
	if (SelectValid())
		::SendMessage(theApp.m_pMainWnd->GetSafeHwnd(),WM_PATH_SELECTED, 0, 0);
	CDialog::OnOK();
}


void CSetPathDlg::OnBnClickedOpenFolder()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	::PostMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_COMMAND, ID_FILE_OPEN_FOLDER, 0);
	OnCancel();			//����ˡ�����·������ť��ر�����·���Ի���
}


void CSetPathDlg::OnPlayPath()
{
	// TODO: �ڴ���������������
	OnOK();
}


void CSetPathDlg::OnDeletePath()
{
	// TODO: �ڴ���������������
	if (SelectValid())
	{
		m_recent_path.erase(m_recent_path.begin() + m_path_selected);	//ɾ��ѡ�е�·��
		ShowPathList();		//������ʾ·���б�
	}
}


void CSetPathDlg::OnBrowsePath()
{
	// TODO: �ڴ���������������
	if (SelectValid())
		ShellExecute(NULL, _T("open"), _T("explorer"), m_recent_path[m_path_selected].path.c_str(), NULL, SW_SHOWNORMAL);
}


void CSetPathDlg::OnClearInvalidPath()
{
	// TODO: �ڴ���������������
	if (MessageBox(CCommon::LoadText(IDS_CLEAR_PATH_INQUARY), NULL, MB_ICONQUESTION | MB_OKCANCEL) == IDCANCEL)
		return;
	int cleard_cnt{};
	for (size_t i{}; i < m_recent_path.size(); i++)
	{
		if (!CCommon::FolderExist(m_recent_path[i].path))
		{
			m_recent_path.erase(m_recent_path.begin() + i);		//ɾ�������ڵ�·��
			i--;
			cleard_cnt++;
		}
	}
	ShowPathList();		//������ʾ·���б�
	CString info;
	info = CCommon::LoadTextFormat(IDS_PATH_CLEAR_COMPLETE, { cleard_cnt });
	MessageBox(info, NULL, MB_ICONINFORMATION | MB_OK);
}


void CSetPathDlg::OnInitMenu(CMenu* pMenu)
{
	CDialog::OnInitMenu(pMenu);

	// TODO: �ڴ˴������Ϣ����������
	bool select_valid{ SelectValid() };
	pMenu->EnableMenuItem(ID_PLAY_PATH, MF_BYCOMMAND | (select_valid ? MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_DELETE_PATH, MF_BYCOMMAND | (select_valid ? MF_ENABLED : MF_GRAYED));
	pMenu->EnableMenuItem(ID_BROWSE_PATH, MF_BYCOMMAND | (select_valid ? MF_ENABLED : MF_GRAYED));
}


void CSetPathDlg::OnEnChangeSearchEdit()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString key_word;
	m_search_edit.GetWindowText(key_word);
	m_searched = (key_word.GetLength() != 0);
	QuickSearch(wstring(key_word));
	ShowPathList();

}


void CSetPathDlg::OnBnClickedClearButton()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_searched)
	{
		//����������
		m_searched = false;
		m_search_edit.SetWindowText(_T(""));
		ShowPathList();
	}
}


BOOL CSetPathDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYDOWN && pMsg->hwnd != m_search_edit.GetSafeHwnd())
	{
		if (pMsg->wParam == 'F')	//��F�����ٲ���
		{
			m_search_edit.SetFocus();
			return TRUE;
		}
	}

	if (pMsg->message == WM_MOUSEMOVE)
		m_Mytip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}
