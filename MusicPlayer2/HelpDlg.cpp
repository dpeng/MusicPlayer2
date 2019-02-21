// HelpDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "MusicPlayer2.h"
#include "HelpDlg.h"
#include "afxdialogex.h"


// CHelpDlg �Ի���

IMPLEMENT_DYNAMIC(CHelpDlg, CDialog)

CHelpDlg::CHelpDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_HELP_DIALOG, pParent)
{

}

CHelpDlg::~CHelpDlg()
{
}

void CHelpDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_HELP_EDIT, m_help_edit);
}


BEGIN_MESSAGE_MAP(CHelpDlg, CDialog)
	ON_WM_GETMINMAXINFO()
END_MESSAGE_MAP()


// CHelpDlg ��Ϣ�������


BOOL CHelpDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  �ڴ���Ӷ���ĳ�ʼ��
	//help_info.LoadString(IDS_HELP_INFO);

	SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);		// ����Сͼ��

	//��ȡ��ʼʱ���ڵĴ�С
	CRect rect;
	GetWindowRect(rect);
	m_min_size.cx = rect.Width();
	m_min_size.cy = rect.Height();

	m_help_edit.SetWindowText(GetHelpString());

	return TRUE;  // return TRUE unless you set the focus to a control
				  // �쳣: OCX ����ҳӦ���� FALSE
}

CString CHelpDlg::GetHelpString()
{
	CString help_info;
	HRSRC hRes;
	if(theApp.m_general_setting_data.language == Language::FOLLOWING_SYSTEM)
		hRes = FindResource(NULL, MAKEINTRESOURCE(IDR_TEXT1), _T("TEXT"));
	else
		hRes = FindResourceEx(NULL, _T("TEXT"), MAKEINTRESOURCE(IDR_TEXT1), theApp.GetCurrentLanguage());
	if (hRes != NULL)
	{
		HGLOBAL hglobal = LoadResource(NULL, hRes);
		if (hglobal != NULL)
			help_info.Format(_T("%s"), (LPVOID)hglobal);
	}

	//�ڰ�����Ϣ��������ϵͳ��Ϣ
	help_info += _T("\r\n\r\nSystem Info:\r\n");

	CString strTmp;
	strTmp.Format(_T("Windows Version: %d.%d build %d\r\n"), CWinVersionHelper::GetMajorVersion(),
		CWinVersionHelper::GetMinorVersion(), CWinVersionHelper::GetBuildNumber());
	help_info += strTmp;

	strTmp.Format(_T("DPI: %d"), theApp.m_dpi);
	help_info += strTmp;
	help_info += _T("\r\n");

	return help_info;
}

void CHelpDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	//���ƴ�����С��С
	lpMMI->ptMinTrackSize.x = m_min_size.cx;		//������С���
	lpMMI->ptMinTrackSize.y = m_min_size.cy;		//������С�߶�

	CDialog::OnGetMinMaxInfo(lpMMI);
}
