// DataSettingsDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "DataSettingsDlg.h"
#include "afxdialogex.h"


// CDataSettingsDlg �Ի���

IMPLEMENT_DYNAMIC(CDataSettingsDlg, CTabDlg)

CDataSettingsDlg::CDataSettingsDlg(CWnd* pParent /*=NULL*/)
	: CTabDlg(IDD_DATA_SETTINGS_DIALOG, pParent)
{

}

CDataSettingsDlg::~CDataSettingsDlg()
{
}

void CDataSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CTabDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SF2_PATH_EDIT, m_sf2_path_edit);
	DDX_Control(pDX, IDC_COMBO1, m_language_combo);
}


BEGIN_MESSAGE_MAP(CDataSettingsDlg, CTabDlg)
	ON_BN_CLICKED(IDC_CLEAN_DATA_FILE_BUTTON, &CDataSettingsDlg::OnBnClickedCleanDataFileButton)
	ON_BN_CLICKED(IDC_ID3V2_FIRST_CHECK, &CDataSettingsDlg::OnBnClickedId3v2FirstCheck)
	ON_BN_CLICKED(IDC_COVER_AUTO_DOWNLOAD_CHECK, &CDataSettingsDlg::OnBnClickedCoverAutoDownloadCheck)
	ON_BN_CLICKED(IDC_LYRIC_AUTO_DOWNLOAD_CHECK, &CDataSettingsDlg::OnBnClickedLyricAutoDownloadCheck)
	ON_BN_CLICKED(IDC_CHECK_UPDATE_CHECK, &CDataSettingsDlg::OnBnClickedCheckUpdateCheck)
	ON_BN_CLICKED(IDC_BROWSE_BUTTON, &CDataSettingsDlg::OnBnClickedBrowseButton)
	ON_BN_CLICKED(IDC_MIDI_USE_INNER_LYRIC_CHECK, &CDataSettingsDlg::OnBnClickedMidiUseInnerLyricCheck)
	ON_BN_CLICKED(IDC_DOWNLOAD_WHEN_TAG_FULL_CHECK, &CDataSettingsDlg::OnBnClickedDownloadWhenTagFullCheck)
	ON_EN_CHANGE(IDC_SF2_PATH_EDIT, &CDataSettingsDlg::OnEnChangeSf2PathEdit)
END_MESSAGE_MAP()


// CDataSettingsDlg ��Ϣ�������


BOOL CDataSettingsDlg::OnInitDialog()
{
	CTabDlg::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	//SetBackgroundColor(RGB(255, 255, 255));

	m_language_combo.AddString(CCommon::LoadText(IDS_FOLLOWING_SYSTEM));
	m_language_combo.AddString(_T("English"));
	m_language_combo.AddString(_T("��������"));
	m_language_combo.SetCurSel(static_cast<int>(m_data.language));

	m_data_size = CCommon::GetFileSize(theApp.m_song_data_path);
	ShowDataSizeInfo();

	((CButton*)GetDlgItem(IDC_ID3V2_FIRST_CHECK))->SetCheck(m_data.id3v2_first);
	((CButton*)GetDlgItem(IDC_COVER_AUTO_DOWNLOAD_CHECK))->SetCheck(m_data.auto_download_album_cover);
	((CButton*)GetDlgItem(IDC_LYRIC_AUTO_DOWNLOAD_CHECK))->SetCheck(m_data.auto_download_lyric);
	((CButton*)GetDlgItem(IDC_DOWNLOAD_WHEN_TAG_FULL_CHECK))->SetCheck(m_data.auto_download_only_tag_full);
	((CButton*)GetDlgItem(IDC_CHECK_UPDATE_CHECK))->SetCheck(m_data.check_update_when_start);
	SetDlgItemText(IDC_SF2_PATH_EDIT, m_data.sf2_path.c_str());
	((CButton*)GetDlgItem(IDC_MIDI_USE_INNER_LYRIC_CHECK))->SetCheck(m_data.midi_use_inner_lyric);
	if (m_data.minimize_to_notify_icon)
		((CButton*)GetDlgItem(IDC_MINIMIZE_TO_NOTIFY_RADIO))->SetCheck(TRUE);
	else
		((CButton*)GetDlgItem(IDC_EXIT_PROGRAM_RADIO))->SetCheck(TRUE);

	m_toolTip.Create(this);
	m_toolTip.SetMaxTipWidth(theApp.DPI(300));
	m_toolTip.AddTool(GetDlgItem(IDC_CLEAN_DATA_FILE_BUTTON), CCommon::LoadText(IDS_CLEAR_DATA_FILE_TIP_INFO));
	m_toolTip.AddTool(GetDlgItem(IDC_DOWNLOAD_WHEN_TAG_FULL_CHECK), CCommon::LoadText(IDS_AUTO_DOWNLOAD_LYRIC_TIP_INFO));
	//m_toolTip.AddTool(GetDlgItem(IDC_SF2_PATH_EDIT), _T("��Ҫ�������ɫ����ܲ��� MIDI ���֡�"));
	m_toolTip.AddTool(GetDlgItem(IDC_MIDI_USE_INNER_LYRIC_CHECK), CCommon::LoadText(IDS_MIDI_INNER_LYRIC_TIP_INFO));

	m_toolTip.SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);

	GetDlgItem(IDC_BROWSE_BUTTON)->EnableWindow(theApp.m_format_convert_dialog_exit);		//���ڽ��и�ʽת��ʱ�����������ɫ��
	m_sf2_path_edit.EnableWindow(theApp.m_format_convert_dialog_exit);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

void CDataSettingsDlg::ShowDataSizeInfo()
{
	CString info;
	if (m_data_size < 1024)
		info.Format(_T("%s: %d %s"), CCommon::LoadText(IDS_CURRENT_DATA_FILE_SIZE), m_data_size, CCommon::LoadText(IDS_BYTE));
	else if (m_data_size < 1024 * 1024)
		info.Format(_T("%s: %.2f KB (%d %s)"), CCommon::LoadText(IDS_CURRENT_DATA_FILE_SIZE), static_cast<float>(m_data_size) / 1024.0f, m_data_size, CCommon::LoadText(IDS_BYTE));
	else
		info.Format(_T("%s: %.2f MB (%d %s)"), CCommon::LoadText(IDS_CURRENT_DATA_FILE_SIZE), static_cast<float>(m_data_size) / 1024.0f / 1024.0f, m_data_size, CCommon::LoadText(IDS_BYTE));		//ע���˴��������ڡ�%.2fMB��©���ˡ�f�����³�����һ������Ի�������ֹͣ�������������⡣
	SetDlgItemText(IDC_SIZE_STATIC, info);
}



void CDataSettingsDlg::OnBnClickedCleanDataFileButton()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	
	CWaitCursor wait_cursor;	//��ʾ�ȴ����
	int clear_cnt{};		//ͳ��ɾ������Ŀ������
	//����ӳ��������ɾ������Ҫ����Ŀ��
	for (auto iter{ theApp.m_song_data.begin() }; iter != theApp.m_song_data.end();)
	{
		//������Ŀ��Ӧ���ļ����ڵ�·���Ƿ��ڡ��������·�����б���
		bool path_exist{ false };	//���iterָ�����Ŀ���ļ�·���ڡ��������·�����б�(CPlayer::GetInstance().GetRecentPath())���Ϊtrue
		wstring item_path;
		size_t index = iter->first.rfind(L'\\');
		item_path = iter->first.substr(0, index + 1);		//��ȡiterָ����Ŀ���ļ�Ŀ¼
		for (int i{}; i < (int)CPlayer::GetInstance().GetRecentPath().size(); i++)
		{
			if (item_path == CPlayer::GetInstance().GetRecentPath()[i].path)
			{
				path_exist = true;
				break;
			}
		}
		//�������Ŀ��Ӧ���ļ����ڵ�·�����ڡ��������·�����б�������Ŀ��Ӧ���ļ������ڣ���ɾ������Ŀ
		if (!path_exist || !CCommon::FileExist(iter->first))
		{
			iter = theApp.m_song_data.erase(iter);		//ɾ����Ŀ֮�󽫵�����ָ��ɾ����Ŀ��ǰһ����Ŀ
			clear_cnt++;
		}
		else
		{
			iter++;
		}
	}
	theApp.SaveSongData();		//���������д���ļ�

	size_t data_size = CCommon::GetFileSize(theApp.m_song_data_path);	 //����������ļ��Ĵ�С
	int size_reduced = m_data_size - data_size;		//����������ļ����ٵ��ֽ���
	if (size_reduced < 0) size_reduced = 0;
	CString info;
	info = CCommon::LoadTextFormat(IDS_CLEAR_COMPLETE_INFO, { clear_cnt, size_reduced });
	MessageBox(info, NULL, MB_ICONINFORMATION);
	m_data_size = data_size;
	ShowDataSizeInfo();
}


void CDataSettingsDlg::OnBnClickedId3v2FirstCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_data.id3v2_first = (((CButton*)GetDlgItem(IDC_ID3V2_FIRST_CHECK))->GetCheck() != 0);
}


void CDataSettingsDlg::OnBnClickedCoverAutoDownloadCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_data.auto_download_album_cover = (((CButton*)GetDlgItem(IDC_COVER_AUTO_DOWNLOAD_CHECK))->GetCheck() != 0);
}


void CDataSettingsDlg::OnBnClickedLyricAutoDownloadCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_data.auto_download_lyric = (((CButton*)GetDlgItem(IDC_LYRIC_AUTO_DOWNLOAD_CHECK))->GetCheck() != 0);
}


void CDataSettingsDlg::OnBnClickedCheckUpdateCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_data.check_update_when_start = (((CButton*)GetDlgItem(IDC_CHECK_UPDATE_CHECK))->GetCheck() != 0);
}


void CDataSettingsDlg::OnBnClickedBrowseButton()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//���ù�����
	CString szFilter = CCommon::LoadText(IDS_SOUND_FONT_FILTER);
	//������ļ��Ի���
	CFileDialog fileDlg(TRUE, _T("SF2"), NULL, 0, szFilter, this);
	//��ʾ���ļ��Ի���
	if (IDOK == fileDlg.DoModal())
	{
		m_data.sf2_path = fileDlg.GetPathName();	//��ȡ�򿪵��ļ�·��
		SetDlgItemText(IDC_SF2_PATH_EDIT, m_data.sf2_path.c_str());
	}
}


BOOL CDataSettingsDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	if (pMsg->message == WM_MOUSEMOVE)
		m_toolTip.RelayEvent(pMsg);

	return CTabDlg::PreTranslateMessage(pMsg);
}


void CDataSettingsDlg::OnBnClickedMidiUseInnerLyricCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_data.midi_use_inner_lyric = (((CButton*)GetDlgItem(IDC_MIDI_USE_INNER_LYRIC_CHECK))->GetCheck() != 0);
}


void CDataSettingsDlg::OnBnClickedDownloadWhenTagFullCheck()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_data.auto_download_only_tag_full = (((CButton*)GetDlgItem(IDC_DOWNLOAD_WHEN_TAG_FULL_CHECK))->GetCheck() != 0);
}


void CDataSettingsDlg::OnEnChangeSf2PathEdit()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CTabDlg::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�
	if (m_sf2_path_edit.GetModify())
	{
		CString str;
		m_sf2_path_edit.GetWindowText(str);
		m_data.sf2_path = str;
	}

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CDataSettingsDlg::OnOK()
{
	// TODO: �ڴ����ר�ô����/����û���

	m_data.minimize_to_notify_icon = (((CButton*)GetDlgItem(IDC_MINIMIZE_TO_NOTIFY_RADIO))->GetCheck() != 0);

	//��ȡ���Ե�����
	m_data.language = static_cast<Language>(m_language_combo.GetCurSel());
	if (m_data.language != theApp.m_general_setting_data.language)
	{
		MessageBox(CCommon::LoadText(IDS_LANGUAGE_CHANGE_INFO), NULL, MB_ICONINFORMATION | MB_OK);
	}

	CTabDlg::OnOK();
}
