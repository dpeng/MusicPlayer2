// LyricDownloadDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "LyricDownloadDlg.h"
#include "afxdialogex.h"
#pragma warning (disable : 4018)


// CLyricDownloadDlg �Ի���

IMPLEMENT_DYNAMIC(CLyricDownloadDlg, CDialog)

CLyricDownloadDlg::CLyricDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_LYRIC_DOWNLOAD_DIALOG, pParent)
{

}

CLyricDownloadDlg::~CLyricDownloadDlg()
{
}


void CLyricDownloadDlg::ShowDownloadList()
{
	m_down_list_ctrl.DeleteAllItems();
	for (int i{}; i < m_down_list.size(); i++)
	{
		CString tmp;
		tmp.Format(_T("%d"), i + 1);
		m_down_list_ctrl.InsertItem(i, tmp);
		m_down_list_ctrl.SetItemText(i, 1, m_down_list[i].title.c_str());
		m_down_list_ctrl.SetItemText(i, 2, m_down_list[i].artist.c_str());
		m_down_list_ctrl.SetItemText(i, 3, m_down_list[i].album.c_str());
	}
}

bool CLyricDownloadDlg::SaveLyric(const wchar_t * path, CodeType code_type)
{
	bool char_connot_convert;
	string lyric_str = CCommon::UnicodeToStr(m_lyric_str, code_type, &char_connot_convert);
	if (char_connot_convert)	//���ļ��а���Unicode�ַ�ʱ��ѯ���û��Ƿ�Ҫѡ��һ��Unicode�����ʽ�ٱ���
	{
		CString info;
		info.LoadString(IDS_STRING103);		//��string table�����ַ���
		if (MessageBox(info, NULL, MB_OKCANCEL | MB_ICONWARNING) != IDOK) return false;		//����û������ȡ����ť���򷵻�false
	}
	ofstream out_put{ path, std::ios::binary };
	out_put << lyric_str;
	return true;
}

void CLyricDownloadDlg::SaveConfig() const
{
	CCommon::WritePrivateProfileIntW(L"lyric_download", L"download_translate", m_download_translate, theApp.m_config_path.c_str());
	CCommon::WritePrivateProfileIntW(L"lyric_download", L"save_as_utf8", static_cast<int>(m_save_code), theApp.m_config_path.c_str());
	CCommon::WritePrivateProfileIntW(L"lyric_download", L"save_to_song_folder", m_save_to_song_folder, theApp.m_config_path.c_str());
	CCommon::WritePrivateProfileIntW(L"lyric_download", L"search_max_item", m_search_max_item, theApp.m_config_path.c_str());
}

void CLyricDownloadDlg::LoadConfig()
{
	m_download_translate = (GetPrivateProfileInt(_T("lyric_download"), _T("download_translate"), 1, theApp.m_config_path.c_str()) != 0);
	m_save_code = static_cast<CodeType>(GetPrivateProfileInt(_T("lyric_download"), _T("save_as_utf8"), 1, theApp.m_config_path.c_str()));
	m_save_to_song_folder = (GetPrivateProfileInt(_T("lyric_download"), _T("save_to_song_folder"), 1, theApp.m_config_path.c_str()) != 0);
	m_search_max_item = GetPrivateProfileInt(_T("lyric_download"), _T("search_max_item"), 30, theApp.m_config_path.c_str());
}

void CLyricDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LYRIC_DOWN_LIST1, m_down_list_ctrl);
	DDX_Control(pDX, IDC_DOWNLOAD_TRANSLATE_CHECK1, m_download_translate_chk);
	DDX_Control(pDX, IDC_COMBO2, m_save_code_combo);
}


BEGIN_MESSAGE_MAP(CLyricDownloadDlg, CDialog)
	ON_BN_CLICKED(IDC_SEARCH_BUTTON2, &CLyricDownloadDlg::OnBnClickedSearchButton2)
	ON_EN_CHANGE(IDC_TITLE_EDIT1, &CLyricDownloadDlg::OnEnChangeTitleEdit1)
	ON_EN_CHANGE(IDC_ARTIST_EDIT1, &CLyricDownloadDlg::OnEnChangeArtistEdit1)
	ON_NOTIFY(NM_CLICK, IDC_LYRIC_DOWN_LIST1, &CLyricDownloadDlg::OnNMClickLyricDownList1)
	ON_NOTIFY(NM_RCLICK, IDC_LYRIC_DOWN_LIST1, &CLyricDownloadDlg::OnNMRClickLyricDownList1)
	ON_BN_CLICKED(IDC_DOWNLOAD_SELECTED, &CLyricDownloadDlg::OnBnClickedDownloadSelected)
	ON_BN_CLICKED(IDC_DOWNLOAD_TRANSLATE_CHECK1, &CLyricDownloadDlg::OnBnClickedDownloadTranslateCheck1)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_SEARCH_COMPLATE, &CLyricDownloadDlg::OnSearchComplate)
	ON_MESSAGE(WM_DOWNLOAD_COMPLATE, &CLyricDownloadDlg::OnDownloadComplate)
	ON_BN_CLICKED(IDC_SAVE_TO_SONG_FOLDER1, &CLyricDownloadDlg::OnBnClickedSaveToSongFolder1)
	ON_BN_CLICKED(IDC_SAVE_TO_LYRIC_FOLDER1, &CLyricDownloadDlg::OnBnClickedSaveToLyricFolder1)
	ON_BN_CLICKED(IDC_SELECTED_SAVE_AS, &CLyricDownloadDlg::OnBnClickedSelectedSaveAs)
	ON_CBN_SELCHANGE(IDC_COMBO2, &CLyricDownloadDlg::OnCbnSelchangeCombo2)
	ON_COMMAND(ID_LD_LYRIC_DOWNLOAD, &CLyricDownloadDlg::OnLdLyricDownload)
	ON_COMMAND(ID_LD_LYRIC_SAVEAS, &CLyricDownloadDlg::OnLdLyricSaveas)
	ON_COMMAND(ID_LD_COPY_TITLE, &CLyricDownloadDlg::OnLdCopyTitle)
	ON_COMMAND(ID_LD_COPY_ARTIST, &CLyricDownloadDlg::OnLdCopyArtist)
	ON_COMMAND(ID_LD_COPY_ALBUM, &CLyricDownloadDlg::OnLdCopyAlbum)
	ON_COMMAND(ID_LD_COPY_ID, &CLyricDownloadDlg::OnLdCopyId)
	ON_COMMAND(ID_LD_VIEW_ONLINE, &CLyricDownloadDlg::OnLdViewOnline)
	ON_NOTIFY(NM_DBLCLK, IDC_LYRIC_DOWN_LIST1, &CLyricDownloadDlg::OnNMDblclkLyricDownList1)
END_MESSAGE_MAP()


// CLyricDownloadDlg ��Ϣ�������


BOOL CLyricDownloadDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	LoadConfig();

	m_title = CPlayer::GetInstance().GetCurrentSongInfo().title;
	m_artist = CPlayer::GetInstance().GetCurrentSongInfo().artist;
	m_album = CPlayer::GetInstance().GetCurrentSongInfo().album;

	if (m_title == CCommon::LoadText(IDS_DEFAULT_TITLE).GetString())		//���û�б�����Ϣ���Ͱ��ļ�����Ϊ����
	{
		m_title = CPlayer::GetInstance().GetFileName();
		size_t index = m_title.rfind(L'.');
		m_title = m_title.substr(0, index);
	}
	if (m_artist == CCommon::LoadText(IDS_DEFAULT_ARTIST).GetString())	//û����������Ϣ����������ҵ��ı�
	{
		m_artist.clear();
	}
	if (m_album == CCommon::LoadText(IDS_DEFAULT_ALBUM).GetString())	//û�г�Ƭ����Ϣ����ճ�Ƭ�����ı�
	{
		m_album.clear();
	}

	const SongInfo& song{ CPlayer::GetInstance().GetCurrentSongInfo() };
	if (!song.is_cue)
		m_file_name = CPlayer::GetInstance().GetFileName();
	else
		m_file_name = song.artist + L" - " + song.title + L".lrc";
	m_file_path = CPlayer::GetInstance().GetCurrentDir() + m_file_name;
	if (!song.is_cue)
	{
		size_t index = m_file_path.rfind(L'.');		//�����ļ������һ����
		m_file_path = m_file_path.substr(0, index + 1) + L"lrc";	//���ļ�������չ����Ϊlrc
	}

	SetDlgItemText(IDC_TITLE_EDIT1, m_title.c_str());
	SetDlgItemText(IDC_ARTIST_EDIT1, m_artist.c_str());

	//�����б�ؼ�������ɫ
	//m_down_list_ctrl.SetColor(theApp.m_app_setting_data.theme_color);

	//��ʼ����������б�ؼ�
	CRect rect;
	m_down_list_ctrl.GetClientRect(rect);
	int width0, width1, width2, width3;
	width0 = rect.Width() / 10;
	width1 = rect.Width() * 3 / 10;
	width2 = rect.Width() * 2 / 10;
	width3 = rect.Width() - theApp.DPI(20) - 1 - width0 - width1 - width2;

	m_down_list_ctrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_LABELTIP);
	m_down_list_ctrl.InsertColumn(0, CCommon::LoadText(IDS_NUMBER), LVCFMT_LEFT, width0);		//�����1��
	m_down_list_ctrl.InsertColumn(1, CCommon::LoadText(IDS_TITLE), LVCFMT_LEFT, width1);		//�����2��
	m_down_list_ctrl.InsertColumn(2, CCommon::LoadText(IDS_ARTIST), LVCFMT_LEFT, width2);		//�����3��
	m_down_list_ctrl.InsertColumn(3, CCommon::LoadText(IDS_ALBUM), LVCFMT_LEFT, width3);		//�����3��

	//�����б�ؼ�����ʾ�����ö������ڽ����������˴��ڵĸ����ھ����ö�����ʱ����ʾ��Ϣ�ڴ������������
	m_down_list_ctrl.GetToolTips()->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	//m_tool_tip.Create(this, TTS_ALWAYSTIP);

	//��ʼ������ѡ���пؼ���״̬
	m_download_translate_chk.SetCheck(m_download_translate);
	m_save_code_combo.AddString(_T("ANSI"));
	m_save_code_combo.AddString(_T("UTF-8"));
	m_save_code_combo.SetCurSel(static_cast<int>(m_save_code));
	if (m_save_to_song_folder)
		((CButton*)GetDlgItem(IDC_SAVE_TO_SONG_FOLDER1))->SetCheck(TRUE);
	else
		((CButton*)GetDlgItem(IDC_SAVE_TO_LYRIC_FOLDER1))->SetCheck(TRUE);

	//�жϸ���ļ����Ƿ����
	bool lyric_path_exist = CCommon::FolderExist(theApp.m_lyric_setting_data.lyric_path);
	if (!lyric_path_exist)		//�������ļ������ڣ�����á����浽����ļ��С���ѡ��ť����ǿ��ѡ�С����浽��������Ŀ¼��
	{
		((CButton*)GetDlgItem(IDC_SAVE_TO_LYRIC_FOLDER1))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_SAVE_TO_LYRIC_FOLDER1))->SetCheck(FALSE);
		((CButton*)GetDlgItem(IDC_SAVE_TO_SONG_FOLDER1))->SetCheck(TRUE);
		m_save_to_song_folder = true;
		//CString info;
		//info.LoadString(IDS_LYRIC_FOLDER_NOT_EXIST);
		//m_tool_tip.AddTool(GetDlgItem(IDC_SAVE_TO_LYRIC_FOLDER1), info);
	}

	//��ʼ���Ҽ��˵�
	m_menu.LoadMenu(IDR_LYRIC_DOWNLOAD_MENU);
	m_menu.GetSubMenu(0)->SetDefaultItem(ID_LD_LYRIC_DOWNLOAD);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}


void CLyricDownloadDlg::OnBnClickedSearchButton2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	SetDlgItemText(IDC_STATIC_INFO, CCommon::LoadText(IDS_SEARCHING));
	GetDlgItem(IDC_SEARCH_BUTTON2)->EnableWindow(FALSE);		//���������������øð�ť
	wstring keyword = CInternetCommon::URLEncode(m_artist + L' ' + m_title);	//�����ؼ���Ϊ�������� ���⡱��������ת����URL����
	wchar_t buff[1024];
	swprintf_s(buff, L"http://music.163.com/api/search/get/?s=%s&limit=%d&type=1&offset=0", keyword.c_str(), m_search_max_item);
	//int rtn = CLyricDownloadCommon::HttpPost(buff, m_search_result);		//�����������ֵĸ�������API����http��POST����
	m_search_thread_info.url = buff;
	m_search_thread_info.hwnd = GetSafeHwnd();
	theApp.m_lyric_download_dialog_exit = false;
	m_pSearchThread = AfxBeginThread(LyricSearchThreadFunc, &m_search_thread_info);
}


void CLyricDownloadDlg::OnEnChangeTitleEdit1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString tmp;
	GetDlgItemText(IDC_TITLE_EDIT1, tmp);
	m_title = tmp;
}


void CLyricDownloadDlg::OnEnChangeArtistEdit1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialog::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString tmp;
	GetDlgItemText(IDC_ARTIST_EDIT1, tmp);
	m_artist = tmp;
}


void CLyricDownloadDlg::OnNMClickLyricDownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_item_selected = pNMItemActivate->iItem;
	*pResult = 0;
}


void CLyricDownloadDlg::OnNMRClickLyricDownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_item_selected = pNMItemActivate->iItem;

	if (m_item_selected >= 0 && m_item_selected < m_down_list.size())
	{
		//�����Ҽ��˵�
		CMenu* pContextMenu = m_menu.GetSubMenu(0);	//��ȡ��һ�������˵�
		CPoint point1;	//����һ������ȷ�����λ�õ�λ��  
		GetCursorPos(&point1);	//��ȡ��ǰ����λ�ã��Ա�ʹ�ò˵����Ը�����
		pContextMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point1.x, point1.y, this); //��ָ��λ����ʾ�����˵�
	}

	*pResult = 0;
}


void CLyricDownloadDlg::OnBnClickedDownloadSelected()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_item_selected < 0 || m_item_selected >= m_down_list.size()) return;
	GetDlgItem(IDC_DOWNLOAD_SELECTED)->EnableWindow(FALSE);		//���������ѡ�������øð�ť
	GetDlgItem(IDC_SELECTED_SAVE_AS)->EnableWindow(FALSE);		//���������ѡ�������øð�ť
	CPlayer::GetInstance().SetRelatedSongID(m_down_list[m_item_selected].id);		//��ѡ����Ŀ�ĸ���ID����������
	m_download_thread_info.hwnd = GetSafeHwnd();
	m_download_thread_info.download_translate = m_download_translate;
	m_download_thread_info.save_as = false;
	m_download_thread_info.song_id = m_down_list[m_item_selected].id;
	m_pDownThread = AfxBeginThread(LyricDownloadThreadFunc, &m_download_thread_info);
}


void CLyricDownloadDlg::OnBnClickedSelectedSaveAs()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if (m_item_selected < 0 || m_item_selected >= m_down_list.size()) return;
	GetDlgItem(IDC_DOWNLOAD_SELECTED)->EnableWindow(FALSE);		//���������ѡ�������øð�ť
	GetDlgItem(IDC_SELECTED_SAVE_AS)->EnableWindow(FALSE);		//���������ѡ�������øð�ť
	m_download_thread_info.hwnd = GetSafeHwnd();
	m_download_thread_info.download_translate = m_download_translate;
	m_download_thread_info.save_as = true;
	m_download_thread_info.song_id = m_down_list[m_item_selected].id;
	m_pDownThread = AfxBeginThread(LyricDownloadThreadFunc, &m_download_thread_info);
}


void CLyricDownloadDlg::OnBnClickedDownloadTranslateCheck1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_download_translate = (m_download_translate_chk.GetCheck() != 0);
}


void CLyricDownloadDlg::OnDestroy()
{
	CDialog::OnDestroy();

	// TODO: �ڴ˴������Ϣ����������
	SaveConfig();
}

UINT CLyricDownloadDlg::LyricSearchThreadFunc(LPVOID lpParam)
{
	CCommon::SetThreadLanguage(theApp.m_general_setting_data.language);
	SearchThreadInfo* pInfo = (SearchThreadInfo*)lpParam;
	wstring result;
	pInfo->rtn = CInternetCommon::HttpPost(pInfo->url, result);		//�����������ֵĸ�������API����http��POST����
	if (theApp.m_lyric_download_dialog_exit) return 0;
	pInfo->result = result;
	::PostMessage(pInfo->hwnd, WM_SEARCH_COMPLATE, 0, 0);		//������ɺ���һ��������ɵ���Ϣ

	return 0;
}

UINT CLyricDownloadDlg::LyricDownloadThreadFunc(LPVOID lpParam)
{
	CCommon::SetThreadLanguage(theApp.m_general_setting_data.language);
	DownloadThreadInfo* pInfo = (DownloadThreadInfo*)lpParam;
	wstring result;
	pInfo->success = CLyricDownloadCommon::DownloadLyric(pInfo->song_id, result, pInfo->download_translate);		//���ظ��
	if (theApp.m_lyric_download_dialog_exit) return 0;
	pInfo->result = result;
	::PostMessage(pInfo->hwnd, WM_DOWNLOAD_COMPLATE, (WPARAM)pInfo->save_as, 0);		//������ɺ���һ�����������Ϣ��wParam���ڴ����Ƿ񵯳������Ϊ���Ի���
	return 0;
}


afx_msg LRESULT CLyricDownloadDlg::OnSearchComplate(WPARAM wParam, LPARAM lParam)
{
	//��ӦWM_SEARCH_CONPLATE��Ϣ
	GetDlgItem(IDC_SEARCH_BUTTON2)->EnableWindow(TRUE);	//�������֮�����øð�ť
	m_search_result = m_search_thread_info.result;
	switch (m_search_thread_info.rtn)
	{
	case 1: MessageBox(CCommon::LoadText(IDS_SEARCH_FAILED_INFO), NULL, MB_ICONWARNING); return 0;
	case 2: MessageBox(CCommon::LoadText(IDS_SEARCH_TIME_OUT), NULL, MB_ICONWARNING); return 0;
	default: break;
	}
	//DEBUGģʽ�£������ҷ��صĽ�����浽�ļ�
#ifdef DEBUG
	ofstream out_put{ L".\\down.log", std::ios::binary };
	out_put << CCommon::UnicodeToStr(m_search_result, CodeType::UTF8);
#endif // DEBUG

	CInternetCommon::DisposeSearchResult(m_down_list, m_search_result);		//�����صĽ��
	ShowDownloadList();			//�������Ľ����ʾ���б�ؼ���

	//����������������ƥ����Ŀ
	int best_matched;
	bool id_releated{ false };
	if (!CPlayer::GetInstance().GetCurrentSongInfo().song_id.empty())		//�����ǰ�����Ѿ��й�����ID������ݸ�ID����������б��в��Ҷ�Ӧ����Ŀ
	{
		for (size_t i{}; i<m_down_list.size(); i++)
		{
			if (CPlayer::GetInstance().GetCurrentSongInfo().song_id == m_down_list[i].id)
			{
				id_releated = true;
				best_matched = i;
				break;
			}
		}
	}
	if(!id_releated)
		best_matched = CInternetCommon::SelectMatchedItem(m_down_list, m_title, m_artist, m_album, m_file_name, true);
	CString info;
	if (m_down_list.empty())
		info = CCommon::LoadText(IDS_SEARCH_NO_SONG);
	else if (best_matched == -1)
		info = CCommon::LoadText(IDS_SEARCH_NO_MATCHED);
	else if(id_releated)
		info = CCommon::LoadTextFormat(IDS_SEARCH_RELATED, { best_matched + 1 });
	else
		info = CCommon::LoadTextFormat(IDS_SEARCH_BEST_MATCHED, { best_matched + 1 });
	SetDlgItemText(IDC_STATIC_INFO, info);
	//�Զ�ѡ���б������ƥ�����Ŀ
	m_down_list_ctrl.SetFocus();
	m_down_list_ctrl.SetItemState(best_matched, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);	//ѡ����
	m_down_list_ctrl.EnsureVisible(best_matched, FALSE);		//ʹѡ���б��ֿɼ�
	m_item_selected = best_matched;
	return 0;
}


void CLyricDownloadDlg::OnCancel()
{
	// TODO: �ڴ����ר�ô����/����û���
	theApp.m_lyric_download_dialog_exit = true;
	if (m_pSearchThread != nullptr)
		WaitForSingleObject(m_pSearchThread->m_hThread, 1000);	//�ȴ��߳��˳�
	if (m_pDownThread != nullptr)
		WaitForSingleObject(m_pDownThread->m_hThread, 1000);	//�ȴ��߳��˳�
	CDialog::OnCancel();
}


void CLyricDownloadDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���
	theApp.m_lyric_download_dialog_exit = true;
	if (m_pSearchThread != nullptr)
		WaitForSingleObject(m_pSearchThread->m_hThread, 1000);	//�ȴ��߳��˳�
	if (m_pDownThread != nullptr)
		WaitForSingleObject(m_pDownThread->m_hThread, 1000);	//�ȴ��߳��˳�
	CDialog::OnOK();
}


afx_msg LRESULT CLyricDownloadDlg::OnDownloadComplate(WPARAM wParam, LPARAM lParam)
{
	m_lyric_str = m_download_thread_info.result;
	GetDlgItem(IDC_DOWNLOAD_SELECTED)->EnableWindow(TRUE);		//������ɺ����øð�ť
	GetDlgItem(IDC_SELECTED_SAVE_AS)->EnableWindow(TRUE);		//������ɺ����øð�ť
	if (!m_download_thread_info.success || m_lyric_str.empty())
	{
		MessageBox(CCommon::LoadText(IDS_LYRIC_DOWNLOAD_FAILED, _T("!")), NULL, MB_ICONWARNING);
		return 0;
	}
	if (!CLyricDownloadCommon::DisposeLryic(m_lyric_str))
	{
		MessageBox(CCommon::LoadText(IDS_SONG_NO_LYRIC, _T("!")), NULL, MB_ICONWARNING);
		return 0;
	}

	CLyricDownloadCommon::AddLyricTag(m_lyric_str, m_down_list[m_item_selected].id, m_down_list[m_item_selected].title, m_down_list[m_item_selected].artist, m_down_list[m_item_selected].album);

	//������
	if (wParam == 0)		//wParamΪ0ʱ�����������Ϊ�Ի���
	{
		wstring saved_path;
		if (m_save_to_song_folder)
		{
			saved_path = m_file_path;
		}
		else
		{
			if (!CCommon::FolderExist(theApp.m_lyric_setting_data.lyric_path))
			{
				CString info;
				info.LoadString(IDS_LYRIC_FOLDER_NOT_EXIST);
				MessageBox(info, NULL, MB_ICONWARNING | MB_OK);
				return 0;
			}
			saved_path = theApp.m_lyric_setting_data.lyric_path + m_file_name;
			size_t index = saved_path.rfind(L'.');		//�����ļ������һ����
			saved_path = saved_path.substr(0, index + 1) + L"lrc";	//���ļ�������չ����Ϊlrc

		}
		if (CCommon::FileExist(saved_path))
		{
			if (MessageBox(CCommon::LoadText(IDS_LYRIC_OVERWRITE_INQUARY), NULL, MB_ICONWARNING | MB_OKCANCEL) == IDCANCEL)
				return 0;
		}
		if (!SaveLyric(saved_path.c_str(), m_save_code))	//������
			return 0;
		if (m_download_translate)
		{
			CLyrics lyrics{ saved_path };		//�򿪱�����ĸ��
			lyrics.DeleteRedundantLyric();		//ɾ������ĸ��
			lyrics.CombineSameTimeLyric();		//����ʷ����ԭʼ��ʺϲ���һ��
			lyrics.SaveLyric2();
		}

		if (m_file_name == CPlayer::GetInstance().GetFileName())		//������ڲ��ŵĸ������ǵ�ǰ���ظ�ʵĸ������Ÿ��¸����ʾ
			CPlayer::GetInstance().IniLyrics(saved_path);
		CString info;
		info = CCommon::LoadTextFormat(IDS_DOWNLOAD_COMPLETE_SAVED, { saved_path });
		MessageBox(info, NULL, MB_ICONINFORMATION);
	}
	else
	{
		//���ù�����
		CString szFilter = CCommon::LoadText(IDS_LYRIC_FILE_FILTER);
		//���챣���ļ��Ի���
		CFileDialog fileDlg(FALSE, _T("lrc"), m_file_path.c_str(), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szFilter, this);
		//Ϊ�����Ϊ���Ի������һ�����ѡ���
		fileDlg.AddComboBox(IDC_SAVE_COMBO_BOX);
		//Ϊ���ѡ��������Ŀ
		fileDlg.AddControlItem(IDC_SAVE_COMBO_BOX, 0, CCommon::LoadText(IDS_SAVE_AS_ANSI));
		fileDlg.AddControlItem(IDC_SAVE_COMBO_BOX, 1, CCommon::LoadText(IDS_SAVE_AS_UTF8));
		//Ϊ���ѡ�������Ĭ��ѡ�е���Ŀ
		fileDlg.SetSelectedControlItem(IDC_SAVE_COMBO_BOX, static_cast<int>(m_save_code));

		//��ʾ�����ļ��Ի���
		if (IDOK == fileDlg.DoModal())
		{
			DWORD selected_item;
			fileDlg.GetSelectedControlItem(IDC_SAVE_COMBO_BOX, selected_item);	//��ȡ�������ʽ����ѡ�е���Ŀ
			CodeType save_code{};
			switch (selected_item)
			{
			case 0: save_code = CodeType::ANSI; break;
			case 1: save_code = CodeType::UTF8; break;
			default: break;
			}
			wstring saved_path{ fileDlg.GetPathName().GetString() };
			SaveLyric(saved_path.c_str(), save_code);	//������
			if (m_download_translate)
			{
				CLyrics lyrics{ saved_path };		//�򿪱�����ĸ��
				lyrics.DeleteRedundantLyric();		//ɾ������ĸ��
				lyrics.CombineSameTimeLyric();		//����ʷ����ԭʼ��ʺϲ���һ��
				lyrics.SaveLyric2();
			}

			if (m_file_name == CPlayer::GetInstance().GetFileName())		//������ڲ��ŵĸ������ǵ�ǰ���ظ�ʵĸ������Ÿ��¸����ʾ
				CPlayer::GetInstance().IniLyrics(saved_path);
		}
	}
	return 0;
}


void CLyricDownloadDlg::OnBnClickedSaveToSongFolder1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_save_to_song_folder = true;
}


void CLyricDownloadDlg::OnBnClickedSaveToLyricFolder1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_save_to_song_folder = false;
}


void CLyricDownloadDlg::OnCbnSelchangeCombo2()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//��ȡ��Ͽ���ѡ�еı����ʽ
	switch (m_save_code_combo.GetCurSel())
	{
	case 1: m_save_code = CodeType::UTF8; break;
	default: m_save_code = CodeType::ANSI; break;
	}
}


void CLyricDownloadDlg::OnLdLyricDownload()
{
	// TODO: �ڴ���������������
	OnBnClickedDownloadSelected();
}


void CLyricDownloadDlg::OnLdLyricSaveas()
{
	// TODO: �ڴ���������������
	OnBnClickedSelectedSaveAs();
}


void CLyricDownloadDlg::OnLdCopyTitle()
{
	// TODO: �ڴ���������������
	if (m_item_selected >= 0 && m_item_selected < m_down_list.size())
	{
		if(!CCommon::CopyStringToClipboard(m_down_list[m_item_selected].title))
			MessageBox(CCommon::LoadText(IDS_COPY_CLIPBOARD_FAILED), NULL, MB_ICONWARNING);
	}
}


void CLyricDownloadDlg::OnLdCopyArtist()
{
	// TODO: �ڴ���������������
	if (m_item_selected >= 0 && m_item_selected < m_down_list.size())
	{
		if (!CCommon::CopyStringToClipboard(m_down_list[m_item_selected].artist))
			MessageBox(CCommon::LoadText(IDS_COPY_CLIPBOARD_FAILED), NULL, MB_ICONWARNING);
	}
}


void CLyricDownloadDlg::OnLdCopyAlbum()
{
	// TODO: �ڴ���������������
	if (m_item_selected >= 0 && m_item_selected < m_down_list.size())
	{
		if (!CCommon::CopyStringToClipboard(m_down_list[m_item_selected].album))
			MessageBox(CCommon::LoadText(IDS_COPY_CLIPBOARD_FAILED), NULL, MB_ICONWARNING);
	}
}


void CLyricDownloadDlg::OnLdCopyId()
{
	// TODO: �ڴ���������������
	if (m_item_selected >= 0 && m_item_selected < m_down_list.size())
	{
		if (!CCommon::CopyStringToClipboard(m_down_list[m_item_selected].id))
			MessageBox(CCommon::LoadText(IDS_COPY_CLIPBOARD_FAILED), NULL, MB_ICONWARNING);
	}
}


void CLyricDownloadDlg::OnLdViewOnline()
{
	// TODO: �ڴ���������������
	if (m_item_selected >= 0 && m_item_selected < m_down_list.size())
	{
		//��ȡ�����������иø��������߽�����ַ
		wstring song_url{ L"http://music.163.com/#/song?id=" + m_down_list[m_item_selected].id };
		//�򿪳�����
		ShellExecute(NULL, _T("open"), song_url.c_str(), NULL, NULL, SW_SHOW);
	}
}

//˫���б���Ŀ������ѡ����Ŀ
void CLyricDownloadDlg::OnNMDblclkLyricDownList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_item_selected = pNMItemActivate->iItem;
	if (m_item_selected >= 0 && m_item_selected < m_down_list.size())
	{
		OnBnClickedDownloadSelected();
	}
	*pResult = 0;
}


BOOL CLyricDownloadDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	//if (pMsg->message == WM_MOUSEMOVE)
	//	m_tool_tip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}
