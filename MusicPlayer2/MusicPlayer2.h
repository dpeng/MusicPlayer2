
// MusicPlayer2.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������
#include "Player.h"
#include "AudioCommon.h"
#include "ColorConvert.h"
#include "DrawCommon.h"
#include "IniHelper.h"
#include "WinVersionHelper.h"
#include "CHotkeyManager.h"
#include "CommonData.h"


// CMusicPlayerApp: 
// �йش����ʵ�֣������ MusicPlayer2.cpp
//

class CMusicPlayerApp : public CWinApp
{
public:
	CMusicPlayerApp();

	//CWinVersionHelper m_win_version;		//��ǰWindows�İ汾
	//CPlayer m_player;

	wstring m_module_dir;		//��ǰ����exe�ļ�����Ŀ¼
	wstring m_local_dir;		//��ǰĿ¼��debugģʽ��Ϊ.\��releaseģʽ��Ϊexe�ļ�����Ŀ¼��
	wstring m_config_path;		//�����ļ���·��
	wstring m_song_data_path;	//�������и�����Ϣ�����ļ���·��
	wstring m_recent_path_dat_path;	//"recent_path.dat"�ļ���·��
	wstring m_desktop_path;		//�����·��
	//wstring m_temp_path;		//��ʱ�ļ��е�·��

	map<wstring, SongInfo> m_song_data;		//�������и�����Ϣ���ݵ�ӳ������������ÿһ����Ƶ�ļ��ľ���·����������ÿһ����Ƶ�ļ�����Ϣ
	vector<DeviceInfo> m_output_devices;	//�����豸����Ϣ

	//����ͼ����Դ

	LyricSettingData m_lyric_setting_data;			//��ѡ�����á��Ի����С�������á��е�����
	ApperanceSettingData m_app_setting_data;		//��ѡ�����á��Ի����С�������á��е�����
	GeneralSettingData m_general_setting_data;		//��ѡ�����á��Ի����С��������á��е�����
	PlaySettingData m_play_setting_data;			//��ѡ�����á��Ի����С��������á��е�����
	NonCategorizedSettingData m_nc_setting_data;	//δ�������������
	GlobalHotKeySettingData m_hot_key_setting_data;	//��ȫ�ֿ�ݼ�������
	CHotkeyManager m_hot_key;

	UIData m_ui_data;
	IconSet m_icon_set;
	FontSet m_font_set;

	volatile bool m_lyric_download_dialog_exit{ true };		//����ָʾ������ضԻ����Ѿ��˳�
	volatile bool m_batch_download_dialog_exit{ true };		//����ָʾ����������ضԻ����Ѿ��˳�
	volatile bool m_cover_download_dialog_exit{ true };		//����ָʾ������ضԻ����Ѿ��˳�
	volatile bool m_format_convert_dialog_exit{ true };		//����ָʾ��ʽ�Ի����Ѿ��˳�

	void SaveSongData() const;		//�����и�����Ϣ�����л��ķ�ʽ���浽�ļ�

	static void CheckUpdate(bool message);
	static UINT CheckUpdateThreadFunc(LPVOID lpParam);	//����ʱ�������̺߳���

	void SaveConfig();
	void LoadConfig();

	void LoadIconResource();

	int DPI(int pixel);		//��һ������ֵ����DPI�任
	int DPI(double pixel);
	//����DPI�任���������봦��
	//round��roundΪ0.5ʱ�������룬roundΪ0.4ʱΪ��������
	int DPIRound(double pixel, double round = 0.5);		//�Խ�������������봦��
	void GetDPIFromWindow(CWnd* pWnd);

	int GetDPI() { return m_dpi; }

	WORD GetCurrentLanguage() const;
	bool IsGlobalMultimediaKeyEnabled() const;

private:
	void LoadSongData();			//���ļ��������л��ķ�ʽ��ȡ���и�����Ϣ

	static LRESULT CALLBACK MultiMediaKeyHookProc(int nCode, WPARAM wParam, LPARAM lParam);

private:
	HHOOK m_multimedia_key_hook = NULL;

// ��д
public:

	int m_dpi{};
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHelp();
};

extern CMusicPlayerApp theApp;