#pragma once
#include "ListCtrlEx.h"
#include "TabDlg.h"

// CPlaySettingsDlg 对话框

class CPlaySettingsDlg : public CTabDlg
{
	DECLARE_DYNAMIC(CPlaySettingsDlg)

public:
	CPlaySettingsDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CPlaySettingsDlg();

	PlaySettingData m_data;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PLAY_SETTING_DIALOG };
#endif
protected:
	//控件变量
	CButton m_stop_when_error_check;
	CButton m_auto_play_when_start_chk;
	CButton m_show_taskbar_progress_check;
	CButton m_show_play_state_icon_chk;
	CComboBox m_output_device_combo;
	CListCtrlEx m_device_info_list;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	void ShowDeviceInfo();

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedStopWhenError();
	afx_msg void OnBnClickedShowTaskbarProgress();
	afx_msg void OnCbnSelchangeOutputDeviceCombo();
	afx_msg void OnBnClickedAutoPlayWhenStartCheck();
	afx_msg void OnBnClickedShowPlayStateIconCheck();
};
