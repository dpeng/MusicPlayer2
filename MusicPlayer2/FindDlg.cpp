// FindDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "FindDlg.h"
#include "afxdialogex.h"
#pragma warning (disable : 4018)


// CFindDlg �Ի���

IMPLEMENT_DYNAMIC(CFindDlg, CDialog)

CFindDlg::CFindDlg(const vector<SongInfo>& playlist, CWnd* pParent /*=NULL*/)
	: CDialog(IDD_FIND_DIALOG, pParent), m_playlist{ playlist }
{

}

CFindDlg::~CFindDlg()
{
}

void CFindDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_FIND_LIST, m_find_result_list);
	DDX_Control(pDX, IDC_FIND_FILE_CHECK, m_find_file_check);
	DDX_Control(pDX, IDC_FIND_TITLE_CHECK, m_find_title_check);
	DDX_Control(pDX, IDC_FIND_ARTIST_CHECK, m_find_artist_check);
	DDX_Control(pDX, IDC_FIND_ALBUM_CHECK, m_find_album_check);
}

void CFindDlg::ShowFindResult()
{
	m_find_result_list.DeleteAllItems();
	CString str;
	if (m_find_current_playlist)
	{
		for (int i{}; i<m_find_result.size(); i++)
		{
			str.Format(_T("%u"), m_find_result[i] + 1);
			m_find_result_list.InsertItem(i, str);
			m_find_result_list.SetItemText(i, 1, m_playlist[m_find_result[i]].file_name.c_str());
			m_find_result_list.SetItemText(i, 2, m_playlist[m_find_result[i]].title.c_str());
			m_find_result_list.SetItemText(i, 3, m_playlist[m_find_result[i]].artist.c_str());
			m_find_result_list.SetItemText(i, 4, m_playlist[m_find_result[i]].album.c_str());
			m_find_result_list.SetItemText(i, 5, (CPlayer::GetInstance().GetCurrentDir() + m_playlist[m_find_result[i]].file_name).c_str());
		}
	}
	else
	{
		for (int i{}; i < m_all_find_result.size(); i++)
		{
			str.Format(_T("%u"), i + 1);
			m_find_result_list.InsertItem(i, str);
			int index = m_all_find_result[i].rfind(L'\\');
			wstring file_name = m_all_find_result[i].substr(index + 1);
			m_find_result_list.SetItemText(i, 1, file_name.c_str());
			m_find_result_list.SetItemText(i, 2, theApp.m_song_data[m_all_find_result[i]].title.c_str());
			m_find_result_list.SetItemText(i, 3, theApp.m_song_data[m_all_find_result[i]].artist.c_str());
			m_find_result_list.SetItemText(i, 4, theApp.m_song_data[m_all_find_result[i]].album.c_str());
			m_find_result_list.SetItemText(i, 5, m_all_find_result[i].c_str());
		}
	}
}

void CFindDlg::ShowFindInfo()
{
	CString str;
	int result_mun;
	if (m_find_current_playlist)
		result_mun = m_find_result.size();
	else
		result_mun = m_all_find_result.size();
	str = CCommon::LoadTextFormat(IDS_FIND_DLG_INFO, { m_key_word, result_mun });
	SetDlgItemText(IDC_FIND_INFO_STATIC, str);
}

void CFindDlg::ClearFindResult()
{
	m_find_result.clear();
}

void CFindDlg::SaveConfig()
{
	m_find_option_data = 0;
	if (m_find_file)
		m_find_option_data |= 0x01;
	if (m_find_title)
		m_find_option_data |= 0x02;
	if (m_find_artist)
		m_find_option_data |= 0x04;
	if (m_find_album)
		m_find_option_data |= 0x08;
	if(m_find_current_playlist)
		m_find_option_data |= 0x10;
	CCommon::WritePrivateProfileIntW(L"config", L"find_option_data", m_find_option_data, theApp.m_config_path.c_str());
}

void CFindDlg::LoadConfig()
{
	m_find_option_data = GetPrivateProfileInt(_T("config"), _T("find_option_data"), 0xff, theApp.m_config_path.c_str());
	m_find_file = (m_find_option_data % 2 != 0);
	m_find_title = ((m_find_option_data >> 1) % 2 != 0);
	m_find_artist = ((m_find_option_data >> 2) % 2 != 0);
	m_find_album = ((m_find_option_data >> 3) % 2 != 0);
	m_find_current_playlist = ((m_find_option_data >> 4) % 2 != 0);
}

int CFindDlg::GetSelectedTrack() const
{
	if (m_item_selected >= 0 && m_item_selected < m_find_result.size())
		return m_find_result[m_item_selected];
	else
		return -1;
}

bool CFindDlg::GetFindCurrentPlaylist() const
{
	return m_find_current_playlist;
}

wstring CFindDlg::GetSelectedSongPath() const
{
	if (m_item_selected >= 0 && m_item_selected < m_all_find_result.size())
		return m_all_find_result[m_item_selected];
	else
		return wstring();
}


BEGIN_MESSAGE_MAP(CFindDlg, CDialog)
	ON_EN_CHANGE(IDC_FIND_EDIT, &CFindDlg::OnEnChangeFindEdit)
	ON_NOTIFY(NM_CLICK, IDC_FIND_LIST, &CFindDlg::OnNMClickFindList)
	ON_BN_CLICKED(IDC_FIND_BUTTON, &CFindDlg::OnBnClickedFindButton)
	ON_NOTIFY(NM_DBLCLK, IDC_FIND_LIST, &CFindDlg::OnNMDblclkFindList)
	ON_BN_CLICKED(IDC_FIND_FILE_CHECK, &CFindDlg::OnBnClickedFindFileCheck)
	ON_BN_CLICKED(IDC_FIND_TITLE_CHECK, &CFindDlg::OnBnClickedFindTitleCheck)
	ON_BN_CLICKED(IDC_FIND_ARTIST_CHECK, &CFindDlg::OnBnClickedFindArtistCheck)
	ON_BN_CLICKED(IDC_FIND_ALBUM_CHECK, &CFindDlg::OnBnClickedFindAlbumCheck)
	ON_BN_CLICKED(IDC_FIND_CURRENT_PLAYLIST_RADIO, &CFindDlg::OnBnClickedFindCurrentPlaylistRadio)
	ON_BN_CLICKED(IDC_FIND_ALL_PLAYLIST_RADIO, &CFindDlg::OnBnClickedFindAllPlaylistRadio)
	ON_WM_GETMINMAXINFO()
	ON_WM_SIZE()
	ON_COMMAND(ID_FD_PLAY, &CFindDlg::OnFdPlay)
	ON_COMMAND(ID_FD_OPEN_FILE_LOCATION, &CFindDlg::OnFdOpenFileLocation)
	ON_NOTIFY(NM_RCLICK, IDC_FIND_LIST, &CFindDlg::OnNMRClickFindList)
	ON_COMMAND(ID_FD_COPY_TEXT, &CFindDlg::OnFdCopyText)
END_MESSAGE_MAP()


// CFindDlg ��Ϣ�������

void CFindDlg::OnEnChangeFindEdit()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString find_string;
	GetDlgItemText(IDC_FIND_EDIT, find_string);
	m_key_word = find_string;
}


void CFindDlg::OnNMClickFindList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	m_item_selected = pNMItemActivate->iItem;		//�������ҽ���б�ʱ����ѡ�е���Ŀ���
	GetDlgItem(IDOK)->EnableWindow(m_item_selected != -1);
	*pResult = 0;
}


void CFindDlg::OnBnClickedFindButton()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (!m_key_word.empty())
	{
		if (m_find_current_playlist)		//���ҵ�ǰ�����б�ʱ����m_playlist�����в���
		{
			m_find_result.clear();
			int index;
			bool find_flag;
			for (int i{ 0 }; i < m_playlist.size(); i++)
			{
				find_flag = false;
				if (m_find_file)
				{
					index = m_playlist[i].file_name.find(m_key_word);
					if (index != string::npos) find_flag = true;
				}
				if (m_find_title)
				{
					index = m_playlist[i].title.find(m_key_word);
					if (index != string::npos) find_flag = true;
				}
				if (m_find_artist)
				{
					index = m_playlist[i].artist.find(m_key_word);
					if (index != string::npos) find_flag = true;
				}
				if (m_find_album)
				{
					index = m_playlist[i].album.find(m_key_word);
					if (index != string::npos) find_flag = true;
				}

				if (find_flag)
					m_find_result.push_back(i);		//����ҵ��ˣ����沥���б��е���Ŀ���
			}
			ShowFindResult();
			if (!m_find_result.empty())
				SetDlgItemText(IDC_FIND_RESULT_STATIC, CCommon::LoadText(IDS_FIND_RESULT, _T(": ")));
			else
				SetDlgItemText(IDC_FIND_RESULT_STATIC, CCommon::LoadText(IDS_NO_RESULT));
		}
		else			//�������в����б�ʱ����theApp.m_song_data�����в���
		{
			m_all_find_result.clear();
			wstring a_result;
			int index;
			bool find_flag;
			for (const auto& item : theApp.m_song_data)
			{
				find_flag = false;
				if (m_find_file)
				{
					int index1;
					index1 = item.first.rfind(L'\\');		//����·�������һ����б��
					index = item.first.find(m_key_word, index1);	//�����һ����б�ܿ�ʼ���ҹؼ���
					if (index != string::npos) find_flag = true;
				}
				if (m_find_title)
				{
					index = item.second.title.find(m_key_word);
					if (index != string::npos) find_flag = true;
				}
				if (m_find_artist)
				{
					index = item.second.artist.find(m_key_word);
					if (index != string::npos) find_flag = true;
				}
				if (m_find_album)
				{
					index = item.second.album.find(m_key_word);
					if (index != string::npos) find_flag = true;
				}
				if (find_flag)
					m_all_find_result.push_back(item.first);		//����ҵ��ˣ��ͱ�������ľ���·��
			}
			ShowFindResult();
			if (!m_all_find_result.empty())
				SetDlgItemText(IDC_FIND_RESULT_STATIC, CCommon::LoadText(IDS_FIND_RESULT, _T(": ")));
			else
				SetDlgItemText(IDC_FIND_RESULT_STATIC, CCommon::LoadText(IDS_NO_RESULT));
		}
		m_item_selected = -1;
		ShowFindInfo();
	}
}


BOOL CFindDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);		// ����Сͼ��

	//����������ɫ
	//m_find_result_list.SetColor(theApp.m_app_setting_data.theme_color);

	//���ò���ѡ�ѡ��ť��״̬
	m_find_file_check.SetCheck(m_find_file);
	m_find_title_check.SetCheck(m_find_title);
	m_find_artist_check.SetCheck(m_find_artist);
	m_find_album_check.SetCheck(m_find_album);

	if (m_find_current_playlist)
		((CButton*)GetDlgItem(IDC_FIND_CURRENT_PLAYLIST_RADIO))->SetCheck(TRUE);
	else
		((CButton*)GetDlgItem(IDC_FIND_ALL_PLAYLIST_RADIO))->SetCheck(TRUE);

	//��ʼ���б�ؼ�
	CRect rect;
	m_find_result_list.GetClientRect(rect);
	m_find_result_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
	int list_width{ rect.Width() - theApp.DPI(20) - 1 };		//�б�ؼ���ȼ�ȥ������ֱ�������Ŀ������
	int width0, width1, width2;
	width0 = theApp.DPI(40);
	width2 = (list_width - width0) / 5;
	width1 = list_width - width0 - width2 * 4;
	m_find_result_list.InsertColumn(0, CCommon::LoadText(IDS_NUMBER), LVCFMT_LEFT, width0);		//�����0��
	m_find_result_list.InsertColumn(1, CCommon::LoadText(IDS_FILE_NAME), LVCFMT_LEFT, width1);		//�����1��
	m_find_result_list.InsertColumn(2, CCommon::LoadText(IDS_TITLE), LVCFMT_LEFT, width2);		//�����2��
	m_find_result_list.InsertColumn(3, CCommon::LoadText(IDS_ARTIST), LVCFMT_LEFT, width2);		//�����3��
	m_find_result_list.InsertColumn(4, CCommon::LoadText(IDS_ALBUM), LVCFMT_LEFT, width2);		//�����4��
	m_find_result_list.InsertColumn(5, CCommon::LoadText(IDS_FILE_PATH), LVCFMT_LEFT, width2);		//�����5��

	ShowFindResult();	//��ʾ����һ�εģ����ҽ��
	ShowFindInfo();

	m_key_word.clear();

	GetDlgItem(IDC_FIND_EDIT)->SetFocus();		//���������õ�������

	//�����б�ؼ�����ʾ�����ö������ڽ����������˴��ڵĸ����ھ����ö�����ʱ����ʾ��Ϣ�ڴ������������
	m_find_result_list.GetToolTips()->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	GetWindowRect(rect);
	m_min_width = rect.Width();
	m_min_height = rect.Height();

	//��ʼ���Ҽ��˵�
	if (m_menu.m_hMenu == NULL)
		m_menu.LoadMenu(IDR_FIND_POPUP_MENU);
	m_menu.GetSubMenu(0)->SetDefaultItem(ID_FD_PLAY);

	GetDlgItem(IDOK)->EnableWindow(FALSE);	//���á�����ѡ����Ŀ����ť������ѡ����һ����Ŀ

	return FALSE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CFindDlg::OnNMDblclkFindList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_item_selected = pNMItemActivate->iItem;
	GetDlgItem(IDOK)->EnableWindow(m_item_selected != -1);

	//˫���б���Ŀ��رնԻ��򲢲���ѡ����Ŀ
	OnFdPlay();
	*pResult = 0;
}


BOOL CFindDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_KEYUP && pMsg->wParam == VK_RETURN)		//���س���ִ����������
	{
		OnBnClickedFindButton();
		return TRUE;
	}
	if (pMsg->wParam == VK_RETURN)		//���ΰ��س����˳�
	{
		return TRUE;
	}

	return CDialog::PreTranslateMessage(pMsg);
}


void CFindDlg::OnBnClickedFindFileCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_find_file = (m_find_file_check.GetCheck() != 0);
}


void CFindDlg::OnBnClickedFindTitleCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_find_title = (m_find_title_check.GetCheck() != 0);
}


void CFindDlg::OnBnClickedFindArtistCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_find_artist = (m_find_artist_check.GetCheck() != 0);
}


void CFindDlg::OnBnClickedFindAlbumCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_find_album = (m_find_album_check.GetCheck() != 0);
}



void CFindDlg::OnBnClickedFindCurrentPlaylistRadio()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_find_current_playlist = true;
}


void CFindDlg::OnBnClickedFindAllPlaylistRadio()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_find_current_playlist = false;
}


void CFindDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//���ƴ�����С��С
	lpMMI->ptMinTrackSize.x = m_min_width;		//������С���
	lpMMI->ptMinTrackSize.y = m_min_height;		//������С�߶�

	CDialog::OnGetMinMaxInfo(lpMMI);
}


void CFindDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	// TODO: �ڴ˴������Ϣ����������
	if (m_find_result_list.m_hWnd != NULL&&nType != SIZE_MINIMIZED)
	{
		//�����б�����Ŀ�Ŀ��
		CRect rect;
		m_find_result_list.GetWindowRect(rect);
		int list_width{ rect.Width() - theApp.DPI(20) - 1 };		//�б�ؼ���ȼ�ȥ������ֱ�������Ŀ������
		int width0, width1, width2;
		width0 = theApp.DPI(40);
		width2 = (list_width - width0) / 5;
		width1 = list_width - width0 - width2 * 4;
		m_find_result_list.SetColumnWidth(1, width1);
		m_find_result_list.SetColumnWidth(2, width2);
		m_find_result_list.SetColumnWidth(3, width2);
		m_find_result_list.SetColumnWidth(4, width2);
		m_find_result_list.SetColumnWidth(5, width2);
	}
}


void CFindDlg::OnFdPlay()
{
	// TODO: �ڴ���������������
	if (m_find_current_playlist)
	{
		if (m_item_selected >= 0 && m_item_selected < m_find_result.size())
			OnOK();
	}
	else
	{
		if (m_item_selected >= 0 && m_item_selected < m_all_find_result.size())
			OnOK();
	}
}


void CFindDlg::OnFdOpenFileLocation()
{
	// TODO: �ڴ���������������
	wstring file;
	if (m_find_current_playlist)
	{
		if (m_item_selected >= 0 && m_item_selected < m_find_result.size())
			file = CPlayer::GetInstance().GetCurrentDir() + m_playlist[m_find_result[m_item_selected]].file_name;
		else
			return;
	}
	else
	{
		if (m_item_selected >= 0 && m_item_selected < m_all_find_result.size())
			file = m_all_find_result[m_item_selected];
		else
			return;
	}
	CString str;
	str.Format(_T("/select,\"%s\""), file.c_str());
	ShellExecute(NULL, _T("open"), _T("explorer"), str, NULL, SW_SHOWNORMAL);
}


void CFindDlg::OnNMRClickFindList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_item_selected = pNMItemActivate->iItem;
	GetDlgItem(IDOK)->EnableWindow(m_item_selected != -1);

	if (m_find_current_playlist && (m_item_selected >= 0 && m_item_selected < m_find_result.size())
		|| !m_find_current_playlist && (m_item_selected >= 0 && m_item_selected < m_all_find_result.size()))
	{
		//��ȡ����������ı�
		int sub_item;
		sub_item = pNMItemActivate->iSubItem;
		m_selected_string = m_find_result_list.GetItemText(m_item_selected, sub_item);
		//�����Ҽ��˵�
		CMenu* pContextMenu = m_menu.GetSubMenu(0);	//��ȡ��һ�������˵�
		CPoint point1;	//����һ������ȷ�����λ�õ�λ��  
		GetCursorPos(&point1);	//��ȡ��ǰ����λ�ã��Ա�ʹ�ò˵����Ը�����
		pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, this); //��ָ��λ����ʾ�����˵�
	}

	*pResult = 0;
}


void CFindDlg::OnFdCopyText()
{
	// TODO: �ڴ���������������
	if (!CCommon::CopyStringToClipboard(wstring(m_selected_string)))
		MessageBox(CCommon::LoadText(IDS_COPY_CLIPBOARD_FAILED), NULL, MB_ICONWARNING);
}
