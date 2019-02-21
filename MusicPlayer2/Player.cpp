#include "stdafx.h"
#include "Player.h"
#include "MusicPlayer2.h"
#pragma warning (disable : 4018)
#pragma warning (disable : 4244)

CBASSMidiLibrary CPlayer::m_bass_midi_lib;
CPlayer CPlayer::m_instance;

CPlayer::CPlayer()
{
}

CPlayer & CPlayer::GetInstance()
{
	return m_instance;
}

CPlayer::~CPlayer()
{
	UnInitBASS();
}

void CPlayer::IniBASS()
{
	//��ȡ��ǰ����Ƶ����豸
	BASS_DEVICEINFO device_info;
	int rtn;
	int device_index{1};
	theApp.m_output_devices.clear();
	DeviceInfo device{};
	device.index = -1;
	device.name = CCommon::LoadText(IDS_DEFAULT_OUTPUT_DEVICE);
	theApp.m_output_devices.push_back(device);
	while (true)
	{
		device = DeviceInfo{};
		rtn = BASS_GetDeviceInfo(device_index, &device_info);
		if (rtn == 0)
			break;
		device.index = device_index;
		if(device_info.name!=nullptr)
			device.name = CCommon::StrToUnicode(string(device_info.name));
		if (device_info.driver != nullptr)
			device.driver = CCommon::StrToUnicode(string(device_info.driver));
		device.flags = device_info.flags;
		theApp.m_output_devices.push_back(device);
		device_index++;
	}

	for (int i{}; i < theApp.m_output_devices.size(); i++)
	{
		if (theApp.m_output_devices[i].name == theApp.m_play_setting_data.output_device)
		{
			theApp.m_play_setting_data.device_selected = i;
			break;
		}
	}

	//��ʼ��BASE��Ƶ��
	BASS_Init(
		theApp.m_output_devices[theApp.m_play_setting_data.device_selected].index,		//�����豸
		44100,//���������44100������ֵ��
		BASS_DEVICE_CPSPEAKERS,//�źţ�BASS_DEVICE_CPSPEAKERS ע��ԭ�����£�
							   /* Use the Windows control panel setting to detect the number of speakers.*/
							   /* Soundcards generally have their own control panel to set the speaker config,*/
							   /* so the Windows control panel setting may not be accurate unless it matches that.*/
							   /* This flag has no effect on Vista, as the speakers are already accurately detected.*/
		theApp.m_pMainWnd->m_hWnd,//���򴰿�,0���ڿ���̨����
		NULL//���ʶ��,0ʹ��Ĭ��ֵ
	);

	//��֧�ֵ��ļ��б����ԭ��֧�ֵ��ļ���ʽ
	CAudioCommon::m_surpported_format.clear();
	SupportedFormat format;
	format.description = CCommon::LoadText(IDS_BASIC_AUDIO_FORMAT);
	format.extensions.push_back(L"mp3");
	format.extensions.push_back(L"wma");
	format.extensions.push_back(L"wav");
	format.extensions.push_back(L"flac");
	format.extensions.push_back(L"ogg");
	format.extensions.push_back(L"oga");
	format.extensions.push_back(L"m4a");
	format.extensions.push_back(L"mp4");
	format.extensions.push_back(L"cue");
	format.extensions.push_back(L"mp2");
	format.extensions.push_back(L"mp1");
	format.extensions.push_back(L"aif");
	format.extensions_list = L"*.mp3;*.wma;*.wav;*.flac;*.ogg;*.oga;*.m4a;*.mp4;*.cue;*.mp2;*.mp1;*.aif";
	CAudioCommon::m_surpported_format.push_back(format);
	CAudioCommon::m_all_surpported_extensions = format.extensions;
	//����BASS���
	wstring plugin_dir;
	plugin_dir = theApp.m_local_dir + L"Plugins\\";
	vector<wstring> plugin_files;
	CCommon::GetFiles(plugin_dir + L"*.dll", plugin_files);		//��ȡPluginsĿ¼�����е�dll�ļ����ļ���
	m_plugin_handles.clear();
	for (const auto& plugin_file : plugin_files)
	{
		//���ز��
		HPLUGIN handle = BASS_PluginLoad((plugin_dir + plugin_file).c_str(), 0);
		m_plugin_handles.push_back(handle);
		//��ȡ���֧�ֵ���Ƶ�ļ�����
		const BASS_PLUGININFO* plugin_info = BASS_PluginGetInfo(handle);
		if (plugin_info == nullptr)
			continue;
		format.file_name = plugin_file;
		format.description = CCommon::ASCIIToUnicode(plugin_info->formats->name);	//���֧���ļ����͵�����
		format.extensions_list = CCommon::ASCIIToUnicode(plugin_info->formats->exts);	//���֧���ļ���չ���б�
		//������չ���б�vector
		format.extensions.clear();
		size_t index = 0, last_index = 0;
		while (true)
		{
			index = format.extensions_list.find(L"*.", index + 1);
			wstring ext{ format.extensions_list.substr(last_index + 2, index - last_index - 2) };
			if (!ext.empty() && ext.back() == L';')
				ext.pop_back();
			format.extensions.push_back(ext);
			if(!CCommon::IsItemInVector(CAudioCommon::m_all_surpported_extensions, ext))
				CAudioCommon::m_all_surpported_extensions.push_back(ext);
			if (index == wstring::npos)
				break;
			last_index = index;
		}
		CAudioCommon::m_surpported_format.push_back(format);

		//����MIDI��ɫ�⣬���ڲ���MIDI
		if (format.description == L"MIDI")
		{
			m_bass_midi_lib.Init(plugin_dir + plugin_file);
			m_sfont_name = CCommon::LoadText(_T("<"), IDS_NONE, _T(">"));
			m_sfont.font = 0;
			if (m_bass_midi_lib.IsSuccessed())
			{
				wstring sf2_path = theApp.m_general_setting_data.sf2_path;
				if (!CCommon::FileExist(sf2_path))		//������õ���ɫ��·�������ڣ����.\Plugins\soundfont\Ŀ¼�²�����ɫ���ļ�
				{
					vector<wstring> sf2s;
					CCommon::GetFiles(plugin_dir + L"soundfont\\*.sf2", sf2s);
					if (!sf2s.empty())
						sf2_path = plugin_dir + L"soundfont\\" + sf2s[0];
				}
				if (CCommon::FileExist(sf2_path))
				{
					m_sfont.font = m_bass_midi_lib.BASS_MIDI_FontInit(sf2_path.c_str(), BASS_UNICODE);
					if (m_sfont.font == 0)
					{
						CString info;
						info = CCommon::LoadTextFormat(IDS_SOUND_FONT_LOAD_FAILED, { sf2_path });
						CCommon::WriteLog((theApp.m_module_dir + L"error.log").c_str(), info.GetString());
						m_sfont_name = CCommon::LoadText(_T("<"), IDS_LOAD_FAILED, _T(">"));
					}
					else
					{
						//��ȡ��ɫ����Ϣ
						BASS_MIDI_FONTINFO sfount_info;
						m_bass_midi_lib.BASS_MIDI_FontGetInfo(m_sfont.font, &sfount_info);
						m_sfont_name = CCommon::StrToUnicode(sfount_info.name);
					}
					m_sfont.preset = -1;
					m_sfont.bank = 0;
				}
			}
		}
	}

}

void CPlayer::UnInitBASS()
{
	BASS_Stop();	//ֹͣ���
	BASS_Free();	//�ͷ�Bass��Դ
	if (m_bass_midi_lib.IsSuccessed() && m_sfont.font != 0)
		m_bass_midi_lib.BASS_MIDI_FontFree(m_sfont.font);
	m_bass_midi_lib.UnInit();
	for (const auto& handle : m_plugin_handles)		//�ͷŲ�����
	{
		BASS_PluginFree(handle);
	}
}

void CPlayer::Create()
{
	IniBASS();
	LoadConfig();
	LoadRecentPath();
	IniPlayList();	//��ʼ�������б�
	//EmplaceCurrentPathToRecent();
	SetTitle();		//�õ�ǰ���ڲ��ŵĸ�������Ϊ���ڱ���
}

void CPlayer::Create(const vector<wstring>& files)
{
	IniBASS();
	LoadConfig();
	LoadRecentPath();
	size_t index;
	index = files[0].find_last_of(L'\\');
	m_path = files[0].substr(0, index + 1);
	SongInfo song_info;
	for (const auto& file : files)
	{
		index = file.find_last_of(L'\\');
		song_info.file_name = file.substr(index + 1);
		m_playlist.push_back(song_info);
	}
	IniPlayList(true);
	//EmplaceCurrentPathToRecent();
	m_current_position_int = 0;
	m_current_position = { 0,0,0 };
	m_index = 0;
	SetTitle();		//�õ�ǰ���ڲ��ŵĸ�������Ϊ���ڱ���
}

void CPlayer::Create(const wstring& path)
{
	IniBASS();
	LoadConfig();
	LoadRecentPath();
	//IniPlayList();	//��ʼ�������б�
	//EmplaceCurrentPathToRecent();
	OpenFolder(path);
	SetTitle();		//�õ�ǰ���ڲ��ŵĸ�������Ϊ���ڱ���
}

void CPlayer::IniPlayList(bool cmd_para, bool refresh_info)
{
	if (!m_loading)
	{
		if (!cmd_para)
		{
			CAudioCommon::GetAudioFiles(m_path, m_playlist, MAX_SONG_NUM);
		}
		//m_index = 0;
		m_song_num = m_playlist.size();
		m_index_tmp = m_index;		//����������
		if (m_index < 0 || m_index >= m_song_num) m_index = 0;		//ȷ����ǰ������Ų��ᳬ����������

		m_loading = true;
		//m_thread_info.playlist = &m_playlist;
		m_thread_info.refresh_info = refresh_info;
		m_thread_info.sort = !cmd_para;
		//m_thread_info.path = m_path;
		m_thread_info.player = this;
		//������ʼ�������б�Ĺ����߳�
		m_pThread = AfxBeginThread(IniPlaylistThreadFunc, &m_thread_info);

		m_song_length = { 0,0,0 };
		//m_current_position = {0,0,0};
		if (m_song_num == 0)
		{
			m_playlist.push_back(SongInfo{});		//û�и���ʱ�򲥷��б����һ���յ�SongInfo����
		}
		m_current_file_name = m_playlist[m_index].file_name;
	}
}

UINT CPlayer::IniPlaylistThreadFunc(LPVOID lpParam)
{
	CCommon::SetThreadLanguage(theApp.m_general_setting_data.language);
	SendMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_PLAYLIST_INI_START, 0, 0);
	ThreadInfo* pInfo = (ThreadInfo*)lpParam;
	//��ȡ�����б���ÿһ�׸�������Ϣ
	//���ֻ��ȡMAX_NUM_LENGTH�׸�ĳ��ȣ�����MAX_NUM_LENGTH�����ĸ����ĳ����ڴ�ʱ��á���ֹ�ļ�������Ƶ�ļ����ർ�µȴ�ʱ�����
	int song_num = pInfo->player->m_playlist.size();
	int song_count = min(song_num, MAX_NUM_LENGTH);
	for (int i{}, count{}; count < song_count && i < song_num; i++)
	{
		pInfo->process_percent = i * 100 / song_count + 1;

		if (!pInfo->refresh_info)
		{
			wstring file_name{ pInfo->player->m_playlist[i].file_name };
			auto iter = theApp.m_song_data.find(pInfo->player->m_path + pInfo->player->m_playlist[i].file_name);
			if (iter != theApp.m_song_data.end())		//���������Ϣ�������Ѿ������ø���������Ҫ�ٻ�ȡ������Ϣ
			{
				pInfo->player->m_playlist[i] = iter->second;
				pInfo->player->m_playlist[i].file_name = file_name;
				continue;
			}
		}
		wstring file_path{ pInfo->player->m_path + pInfo->player->m_playlist[i].file_name };
		HSTREAM hStream;
		hStream = BASS_StreamCreateFile(FALSE, file_path.c_str(), 0, 0, BASS_SAMPLE_FLOAT);
		pInfo->player->AcquireSongInfo(hStream, file_path, pInfo->player->m_playlist[i]);
		BASS_StreamFree(hStream);
		count++;
	}
	pInfo->player->m_loading = false;
	pInfo->player->IniPlaylistComplate(pInfo->sort);
	//pInfo->player->IniLyrics();
	PostMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_PLAYLIST_INI_COMPLATE, 0, 0);
	return 0;
}

void CPlayer::IniPlaylistComplate(bool sort)
{
	CAudioCommon::CheckCueFiles(m_playlist, m_path);
	CAudioCommon::GetCueTracks(m_playlist, m_path);
	m_song_num = m_playlist.size();
	m_index = m_index_tmp;
	if (m_index < 0 || m_index >= m_song_num) m_index = 0;		//ȷ����ǰ������Ų��ᳬ����������
	//ͳ���б���ʱ��
	m_total_time = 0;
	for (const auto& somg : m_playlist)
	{
		m_total_time += somg.lengh.time2int();
	}

	//�Բ����б�����
	if (sort && m_playlist.size() > 1)
		SortPlaylist(false);

	SearchLyrics();

	if (m_song_num > 0)
	{
		if (m_playing == 0)		//�����б��ʼ����ɣ���������ɺ������ʱû���ڲ��ţ����������ò��ŵ��ļ�
		{
			if (!m_current_file_name_tmp.empty())		//���ִ����ReloadPlaylist��m_current_file_name_tmp��Ϊ�գ������m_current_file_name_tmp�������Ŀ������
			{
				//�������벥���б�󣬲������ڲ�����Ŀ�����
				MusicControl(Command::CLOSE);
				for (int i{}; i < m_playlist.size(); i++)
				{
					if (m_current_file_name_tmp == m_playlist[i].file_name)
					{
						m_index = i;
						m_current_file_name = m_current_file_name_tmp;
						break;
					}
				}
				m_current_file_name_tmp.clear();
				MusicControl(Command::OPEN);
				MusicControl(Command::SEEK);
				//MusicControl(Command::PLAY);
			}
			else		//����ֱ�Ӵ򿪲��ŵ�index����Ŀ
			{
				MusicControl(Command::CLOSE);
				m_current_file_name = m_playlist[m_index].file_name;
				MusicControl(Command::OPEN);
				MusicControl(Command::SEEK);
				if(theApp.m_play_setting_data.auto_play_when_start)
					MusicControl(Command::PLAY);
			}
		}
		else		//����û��ڲ��ų�ʼ���Ĺ����н����˲��ţ���������ڲ��ŵ��ļ������²������ڲ��ŵ����
		{
			for (int i{}; i < m_playlist.size(); i++)
			{
				if (m_current_file_name == m_playlist[i].file_name)
				{
					m_index = i;
					break;
				}
			}
		}
	}
	if(!sort)		//����ļ���ͨ�������в����򿪵ģ���sort��Ϊfalse����ʱ�򿪺�ֱ�Ӳ���
		MusicControl(Command::PLAY);

	EmplaceCurrentPathToRecent();
	SetTitle();
	m_shuffle_list.clear();
	if (m_repeat_mode == RM_PLAY_SHUFFLE)
		m_shuffle_list.push_back(m_index);
}

void CPlayer::SearchLyrics(/*bool refresh*/)
{
	//��������ļ�
	//���������ģ��ƥ�䣬�Ƚ����еĸ���ļ����ļ������浽�������Թ�ģ��ƥ��ʱ����
	if (theApp.m_lyric_setting_data.lyric_fuzzy_match)
	{
		m_current_path_lyrics.clear();
		m_lyric_path_lyrics.clear();
		CAudioCommon::GetLyricFiles(m_path, m_current_path_lyrics);
		CAudioCommon::GetLyricFiles(theApp.m_lyric_setting_data.lyric_path, m_lyric_path_lyrics);
	}

	//���������б���ÿһ�׸����ĸ���ļ�����������ļ�·�����浽�б���
	for (auto& song : m_playlist)
	{
		if (song.file_name.size() < 3) continue;
		song.lyric_file.clear();		//�������ǰ�����֮ǰ�Ѿ��������ĸ��
		//if (!song.lyric_file.empty() && CCommon::FileExist(song.lyric_file))		//���������Ϣ���и���ļ����Ҹ���ļ����ڣ�����Ҫ�ٻ�ȡ���
		//	continue;
		CFilePathHelper lyric_path{ m_path + song.file_name };		//�õ�·��+�ļ������ַ���
		lyric_path.ReplaceFileExtension(L"lrc");		//���ļ���չ�滻��lrc
		CFilePathHelper lyric_path2{ theApp.m_lyric_setting_data.lyric_path + song.file_name };
		lyric_path2.ReplaceFileExtension(L"lrc");
		//���Ҹ���ļ����͸����ļ�����ȫƥ��ĸ��
		if (CCommon::FileExist(lyric_path.GetFilePath()))
		{
			song.lyric_file = lyric_path.GetFilePath();
		}
		else if (CCommon::FileExist(lyric_path2.GetFilePath()))		//��ǰĿ¼��û�ж�Ӧ�ĸ���ļ�ʱ������theApp.m_lyric_setting_data.m_lyric_pathĿ¼��Ѱ�Ҹ���ļ�
		{
			song.lyric_file = lyric_path2.GetFilePath();
		}
		else if (theApp.m_lyric_setting_data.lyric_fuzzy_match)
		{
			wstring matched_lyric;		//ƥ��ĸ�ʵ�·��
			//��Ѱ�Ҹ���ļ���ͬʱ������������������ҵĸ���ļ�
			for (const auto& str : m_current_path_lyrics)	//�ڵ�ǰĿ¼��Ѱ��
			{
				//if (str.find(song.artist) != string::npos && str.find(song.title) != string::npos)
				if (CCommon::StringNatchWholeWord(str, song.artist) != -1 && CCommon::StringNatchWholeWord(str, song.title) != -1)
				{
					matched_lyric = m_path + str;
					break;
				}
			}

			if (matched_lyric.empty())		//�����ǰĿ¼��û�ҵ�
			{
				for (const auto& str : m_lyric_path_lyrics)	//�ڸ��Ŀ¼��Ѱ��
				{
					//if (str.find(song.artist) != string::npos && str.find(song.title) != string::npos)
					if (CCommon::StringNatchWholeWord(str, song.artist) != -1 && CCommon::StringNatchWholeWord(str, song.title) != -1)
					{
						matched_lyric = theApp.m_lyric_setting_data.lyric_path + str;
						break;
					}
				}
			}

			//û���ҵ��Ļ���Ѱ�Ҹ���ļ���ֻ������������ĸ���ļ�
			if (matched_lyric.empty())
			{
				for (const auto& str : m_current_path_lyrics)	//�ڵ�ǰĿ¼��Ѱ��
				{
					//if (str.find(song.title) != string::npos)
					if (CCommon::StringNatchWholeWord(str, song.title) != -1)
					{
						matched_lyric = m_path + str;
						break;
					}
				}
			}

			if (matched_lyric.empty())
			{
				for (const auto& str : m_lyric_path_lyrics)	//�ڸ��Ŀ¼��Ѱ��
				{
					//if (str.find(song.title) != string::npos)
					if (CCommon::StringNatchWholeWord(str, song.title) != -1)
					{
						matched_lyric = theApp.m_lyric_setting_data.lyric_path + str;
						break;
					}
				}
			}

			if (!matched_lyric.empty())
				song.lyric_file = matched_lyric;
		}
		////����Ѿ���ȡ���˸�ʣ��򽫸��·�����浽���и�����Ϣ������
		//auto iter = theApp.m_song_data.find(m_path + song.file_name);
		//if (iter != theApp.m_song_data.end())
		//	iter->second.lyric_file = song.lyric_file;
	}
}

void CPlayer::IniLyrics()
{
	if (!m_playlist.empty() && !m_playlist[m_index].lyric_file.empty())
		m_Lyrics = CLyrics{ m_playlist[m_index].lyric_file };
	else
		m_Lyrics = CLyrics{};
}

void CPlayer::IniLyrics(const wstring& lyric_path)
{
	m_Lyrics = CLyrics{ lyric_path };
	m_playlist[m_index].lyric_file = lyric_path;
}

void CPlayer::MidiLyricSync(HSYNC handle, DWORD channel, DWORD data, void * user)
{
	if (!CPlayer::m_bass_midi_lib.IsSuccessed())
		return;
	CPlayer::GetInstance().m_midi_no_lyric = false;
	BASS_MIDI_MARK mark;
	CPlayer::m_bass_midi_lib.BASS_MIDI_StreamGetMark(channel, (DWORD)user, data, &mark); // get the lyric/text
	if (mark.text[0] == '@') return; // skip info
	if (mark.text[0] == '\\')
	{ // clear display
		CPlayer::GetInstance().m_midi_lyric.clear();
	}
	else if (mark.text[0] == '/')
	{
		CPlayer::GetInstance().m_midi_lyric += L"\r\n";
		const char* text = mark.text + 1;
		CPlayer::GetInstance().m_midi_lyric += CCommon::StrToUnicode(text, CodeType::ANSI);
	}
	else
	{
		CPlayer::GetInstance().m_midi_lyric += CCommon::StrToUnicode(mark.text, CodeType::ANSI);
	}
}

void CPlayer::MidiEndSync(HSYNC handle, DWORD channel, DWORD data, void * user)
{
	CPlayer::GetInstance().m_midi_lyric.clear();
}

void CPlayer::MusicControl(Command command, int volume_step)
{
	switch (command)
	{
	case Command::OPEN:
		m_error_code = 0;
		m_musicStream = BASS_StreamCreateFile(FALSE, (m_path + m_current_file_name).c_str(), 0, 0, BASS_SAMPLE_FLOAT);
		BASS_ChannelGetInfo(m_musicStream, &m_channel_info);
		m_is_midi = (CAudioCommon::GetAudioTypeByBassChannel(m_channel_info.ctype) == AudioType::AU_MIDI);
		if (m_bass_midi_lib.IsSuccessed() && m_is_midi && m_sfont.font != 0)
			m_bass_midi_lib.BASS_MIDI_StreamSetFonts(m_musicStream, &m_sfont, 1);
		//��ȡ��Ƶ����
		m_current_file_type = CAudioCommon::GetBASSChannelDescription(m_channel_info.ctype);		//����ͨ����Ϣ��ȡ��ǰ��Ƶ�ļ�������
		if (m_current_file_type.empty())		//�����ȡ������Ƶ�ļ������ͣ������ļ���չ����Ϊ�ļ�����
		{
			CFilePathHelper file_path{ m_current_file_name };
			m_current_file_type = file_path.GetFileExtension(true);
		}
		if (m_song_num > 0)
		{
			if (!m_playlist[m_index].info_acquired)	//�����ǰ�򿪵��ļ�û���ڳ�ʼ�������б�ʱ�����Ϣ�����ʱ���»�ȡ
				AcquireSongInfo(m_musicStream, m_path + m_current_file_name, m_playlist[m_index]);
			m_song_length = m_playlist[m_index].lengh;
			m_song_length_int = m_song_length.time2int();
			//����ļ���MIDI���֣����ʱ��ȡMIDI���ֵ���Ϣ
			if (m_is_midi && m_bass_midi_lib.IsSuccessed())
			{
				//��ȡMIDI������Ϣ
				BASS_ChannelGetAttribute(m_musicStream, BASS_ATTRIB_MIDI_PPQN, &m_midi_info.ppqn); // get PPQN value
				m_midi_info.midi_length = BASS_ChannelGetLength(m_musicStream, BASS_POS_MIDI_TICK) / m_midi_info.ppqn;
				m_midi_info.tempo = m_bass_midi_lib.BASS_MIDI_StreamGetEvent(m_musicStream, 0, MIDI_EVENT_TEMPO);
				m_midi_info.speed = 60000000 / m_midi_info.tempo;
				//��ȡMIDI������Ƕ���
				BASS_MIDI_MARK mark;
				m_midi_lyric.clear();
				if (m_bass_midi_lib.BASS_MIDI_StreamGetMark(m_musicStream, BASS_MIDI_MARK_LYRIC, 0, &mark)) // got lyrics
					BASS_ChannelSetSync(m_musicStream, BASS_SYNC_MIDI_MARK, BASS_MIDI_MARK_LYRIC, MidiLyricSync, (void*)BASS_MIDI_MARK_LYRIC);
				else if (m_bass_midi_lib.BASS_MIDI_StreamGetMark(m_musicStream, BASS_MIDI_MARK_TEXT, 20, &mark)) // got text instead (over 20 of them)
					BASS_ChannelSetSync(m_musicStream, BASS_SYNC_MIDI_MARK, BASS_MIDI_MARK_TEXT, MidiLyricSync, (void*)BASS_MIDI_MARK_TEXT);
				BASS_ChannelSetSync(m_musicStream, BASS_SYNC_END, 0, MidiEndSync, 0);
				m_midi_no_lyric = true;
			}
			//��ʱ��ȡר������
			SearchAlbumCover();
			//��ʼ�����
			IniLyrics();
		}
		if (m_playlist[m_index].is_cue)
		{
			//SeekTo(0);
			m_song_length = GetCurrentSongInfo().lengh;
			m_song_length_int = m_song_length.time2int();
		}
		SetVolume();
		memset(m_spectral_data, 0, sizeof(m_spectral_data));		//���ļ�ʱ���Ƶ�׷���������
		SetFXHandle();
		if (m_equ_enable)
			SetAllEqualizer();
		if (m_reverb_enable)
			SetReverb(m_reverb_mix, m_reverb_time);
		else
			ClearReverb();
		PostMessage(theApp.m_pMainWnd->m_hWnd, WM_MUSIC_STREAM_OPENED, 0, 0);
		break;
	case Command::PLAY:
		ConnotPlayWarning();
		BASS_ChannelPlay(m_musicStream, FALSE); m_playing = 2;
		break;
	case Command::CLOSE:
		RemoveFXHandle();
		BASS_StreamFree(m_musicStream);
		m_playing = 0;
		break;
	case Command::PAUSE: BASS_ChannelPause(m_musicStream); m_playing = 1; break;
	case Command::STOP:
		BASS_ChannelStop(m_musicStream);
		m_playing = 0;
		SeekTo(0);
		memset(m_spectral_data, 0, sizeof(m_spectral_data));		//ֹͣʱ���Ƶ�׷���������
		//GetBASSCurrentPosition();
		break;
	case Command::FF:		//���
		GetBASSCurrentPosition();		//��ȡ��ǰλ�ã����룩
		m_current_position_int += 5000;		//ÿ�ο��5000����
		if (m_current_position_int > m_song_length_int) m_current_position_int -= 5000;
		SeekTo(m_current_position_int);
		break;
	case Command::REW:		//����
		GetBASSCurrentPosition();		//��ȡ��ǰλ�ã����룩
		m_current_position_int -= 5000;		//ÿ�ο���5000����
		if (m_current_position_int < 0) m_current_position_int = 0;		//��ֹ���˵�����λ��
		SeekTo(m_current_position_int);
		break;
	case Command::PLAY_PAUSE:
		if (m_playing == 2)
		{
			BASS_ChannelPause(m_musicStream);
			m_playing = 1;
		}
		else
		{
			ConnotPlayWarning();
			BASS_ChannelPlay(m_musicStream, FALSE);
			m_playing = 2;
		}
		break;
	case Command::VOLUME_UP:
		if (m_volume < 100)
		{
			m_volume += volume_step;
			if (m_volume > 100) m_volume = 100;
			SetVolume();
			SaveConfig();
		}
		break;
	case Command::VOLUME_DOWN:
		if (m_volume > 0)
		{
			m_volume -= volume_step;
			if (m_volume < 0) m_volume = 0;
			SetVolume();
			SaveConfig();
		}
		break;
	case Command::SEEK:		//��λ��m_current_position��λ��
		if (m_current_position_int > m_song_length_int)
		{
			m_current_position_int = 0;
			m_current_position = Time{ 0, 0, 0 };
		}
		SeekTo(m_current_position_int);
		break;
	default: break;
	}
}

bool CPlayer::SongIsOver() const
{
	if (GetCurrentSongInfo().is_cue)
	{
		return (m_playing == 2 && m_current_position_int >= m_song_length_int);
	}
	else
	{
		bool song_is_over;
		static int last_pos;
		if ((m_playing == 2 && m_current_position_int == last_pos && m_current_position_int != 0	//������ڲ����ҵ�ǰ���ŵ�λ��û�з����仯�ҵ�ǰ����λ�ò�Ϊ0��
			&& m_current_position_int > m_song_length_int-2000)		//�Ҳ��Ž��ȵ������2��
			|| m_error_code == BASS_ERROR_ENDED)	//���߳���BASS_ERROR_ENDED�������жϵ�ǰ������������
			//��ʱ������ʶ��ĸ������ȳ���ʵ�ʸ������ȵ����⣬�����ᵼ�¸������Ž��ȳ���ʵ�ʸ�����βʱ�����BASS_ERROR_ENDED����
			//��⵽�������ʱֱ���жϸ����Ѿ��������ˡ�
			song_is_over = true;
		else
			song_is_over = false;
		last_pos = m_current_position_int;
		return song_is_over;
		//���ﱾ��ֱ��ʹ��return m_current_position_int>=m_song_length_int���жϸ����������ˣ�
		//����BASS��Ƶ���ڲ���ʱ���ܻ���ֵ�ǰ����λ��һֱ�޷������������λ�õ����⣬
		//���������ͻ�һֱ����false��
	}
}

void CPlayer::GetBASSSongLength()
{
	QWORD lenght_bytes;
	lenght_bytes = BASS_ChannelGetLength(m_musicStream, BASS_POS_BYTE);
	double length_sec;
	length_sec = BASS_ChannelBytes2Seconds(m_musicStream, lenght_bytes);
	m_song_length_int = static_cast<int>(length_sec * 1000);
	if (m_song_length_int == -1000) m_song_length_int = 0;
	m_song_length.int2time(m_song_length_int);		//������ת����Time�ṹ
}

Time CPlayer::GetBASSSongLength(HSTREAM hStream)
{
	QWORD lenght_bytes;
	lenght_bytes = BASS_ChannelGetLength(hStream, BASS_POS_BYTE);
	double length_sec;
	length_sec = BASS_ChannelBytes2Seconds(hStream, lenght_bytes);
	int song_length_int = static_cast<int>(length_sec * 1000);
	if (song_length_int == -1000) song_length_int = 0;
	return Time(song_length_int);		//������ת����Time�ṹ
}

void CPlayer::GetBASSCurrentPosition()
{
	QWORD pos_bytes;
	pos_bytes = BASS_ChannelGetPosition(m_musicStream, BASS_POS_BYTE);
	double pos_sec;
	pos_sec = BASS_ChannelBytes2Seconds(m_musicStream, pos_bytes);
	m_current_position_int = static_cast<int>(pos_sec * 1000);
	if (m_current_position_int == -1000) m_current_position_int = 0;
	if (m_playlist[m_index].is_cue)
	{
		m_current_position_int -= m_playlist[m_index].start_pos.time2int();
	}
	m_current_position.int2time(m_current_position_int);
	GetMidiPosition();
}

int CPlayer::GetBASSCurrentPosition(HSTREAM hStream)
{
	QWORD pos_bytes;
	pos_bytes = BASS_ChannelGetPosition(hStream, BASS_POS_BYTE);
	double pos_sec;
	pos_sec = BASS_ChannelBytes2Seconds(hStream, pos_bytes);
	return static_cast<int>(pos_sec * 1000);
}


void CPlayer::SetVolume()
{
	float volume = static_cast<float>(m_volume) / 100.0f;
	volume = volume * theApp.m_nc_setting_data.volume_map / 100;
	BASS_ChannelSetAttribute(m_musicStream, BASS_ATTRIB_VOL, volume);
}


void CPlayer::GetBASSSpectral()
{
	if (m_musicStream && m_playing != 0 && m_current_position_int < m_song_length_int - 500)	//ȷ����Ƶ�����Ϊ�գ����Ҹ������500���벻��ʾƵ�ף��Է�ֹ��������ĩβ�޷���ȡƵ�׵Ĵ���
	{
		BASS_ChannelGetData(m_musicStream, m_fft, BASS_DATA_FFT256);
		memset(m_spectral_data, 0, sizeof(m_spectral_data));
		for (int i{}; i < FFT_SAMPLE; i++)
		{
			m_spectral_data[i / (FFT_SAMPLE / SPECTRUM_ROW)] += m_fft[i];
		}

		for (int i{}; i < SPECTRUM_ROW; i++)
		{
			m_spectral_data[i] /= (FFT_SAMPLE / SPECTRUM_ROW);
			m_spectral_data[i] = std::sqrtf(m_spectral_data[i]);		//��ÿ��Ƶ�����ε�ֵȡƽ�������Լ��ٲ�ͬƵ��Ƶ��ֵ�Ĳ���
			m_spectral_data[i] *= 60;			//��������ĳ������Ե���Ƶ�׷�������ͼ����ĸ߶�
		}
	}
	else
	{
		memset(m_spectral_data, 0, sizeof(m_spectral_data));
	}
	//����Ƶ�׶��˵ĸ߶�
	if (m_playing != 1)
	{
		static int fall_count;
		for (int i{}; i < SPECTRUM_ROW; i++)
		{
			if (m_spectral_data[i] > m_last_spectral_data[i])
			{
				m_spectral_peak[i] = m_spectral_data[i];		//�����ǰ��Ƶ�ױ���һ�ε�Ƶ�׸ߣ���Ƶ�׶��˸߶���Ϊ��ǰƵ�׵ĸ߶�
				fall_count = 0;
			}
			else
			{
				fall_count++;
				m_spectral_peak[i] -= (fall_count*0.2);		//�����ǰƵ�ױ���һ�ε�Ƶ�����ͣ���Ƶ�׶��˵ĸ߶����½�
			}
		}
	}

	memcpy_s(m_last_spectral_data, sizeof(m_last_spectral_data), m_spectral_data, sizeof(m_spectral_data));
}


int CPlayer::GetCurrentSecond()
{
	return m_current_position.sec;
}

bool CPlayer::IsPlaying() const
{
	return m_playing == 2;
}

bool CPlayer::PlayTrack(int song_track)
{
	switch (m_repeat_mode)
	{
	case RM_PLAY_ORDER:		//˳�򲥷�
		if (song_track == NEXT)		//������һ��
			song_track = m_index + 1;
		if (song_track == PREVIOUS)		//������һ��
			song_track = m_index - 1;
		break;
	case RM_PLAY_SHUFFLE:		//�������
		if (song_track == NEXT)
		{
			SYSTEMTIME current_time;
			GetLocalTime(&current_time);			//��ȡ��ǰʱ��
			srand(current_time.wMilliseconds);		//�õ�ǰʱ��ĺ��������ò��������������
			song_track = rand() % m_song_num;
			m_shuffle_list.push_back(song_track);	//����������Ź�����Ŀ
		}
		else if (song_track == PREVIOUS)		//������һ�����������Ŀ
		{
			if (m_shuffle_list.size() >= 2)
			{
				if (m_index == m_shuffle_list.back())
					m_shuffle_list.pop_back();
				song_track = m_shuffle_list.back();
			}
			else
			{
				MusicControl(Command::STOP);	//�޷�����ʱֹͣ����
				return true;
			}
		}
		//else if (song_track >= 0 && song_track < m_song_num)
		//{
		//	m_shuffle_list.push_back(song_track);	//����������Ź�����Ŀ
		//}
		break;
	case RM_LOOP_PLAYLIST:		//�б�ѭ��
		if (song_track == NEXT)		//������һ��
		{
			song_track = m_index + 1;
			if (song_track >= m_song_num) song_track = 0;
			if (song_track < 0) song_track = m_song_num - 1;
		}
		if (song_track == PREVIOUS)		//������һ��
		{
			song_track = m_index - 1;
			if (song_track >= m_song_num) song_track = 0;
			if (song_track < 0) song_track = m_song_num - 1;
		}
		break;
	case RM_LOOP_TRACK:		//����ѭ��
		if (song_track == NEXT || song_track == PREVIOUS)
			song_track = m_index;
	}

	if (song_track >= 0 && song_track < m_song_num)
	{
		MusicControl(Command::CLOSE);
		m_index = song_track;
		m_current_file_name = m_playlist[m_index].file_name;
		MusicControl(Command::OPEN);
		//IniLyrics();
		if (m_playlist[m_index].is_cue)
			SeekTo(0);
		MusicControl(Command::PLAY);
		GetBASSCurrentPosition();
		SetTitle();
		SaveConfig();
		EmplaceCurrentPathToRecent();
		SaveRecentPath();
		return true;
	}
	else
	{
		MusicControl(Command::CLOSE);
		m_index = 0;
		m_current_file_name = m_playlist[m_index].file_name;
		MusicControl(Command::OPEN);
		//IniLyrics();
		GetBASSCurrentPosition();
		SetTitle();
		SaveConfig();
		EmplaceCurrentPathToRecent();
		SaveRecentPath();
	}
	return false;
}

void CPlayer::ChangePath(const wstring& path, int track)
{
	if (m_loading) return;
	MusicControl(Command::CLOSE);
	m_path = path;
	if (m_path.empty() || (m_path.back() != L'/' && m_path.back() != L'\\'))		//����������·��Ϊ�ջ�ĩβû��б�ܣ�����ĩβ����һ��
		m_path.append(1, L'\\');
	m_playlist.clear();		//��ղ����б�
	m_index = track;
	//��ʼ�������б�
	IniPlayList();		//������·�����³�ʼ�������б�
	m_current_position_int = 0;
	m_current_position = { 0, 0, 0 };
	SaveConfig();
	SetTitle();
	//MusicControl(Command::OPEN);
	//IniLyrics();
}

void CPlayer::SetPath(const wstring& path, int track, int position, SortMode sort_mode)
{
	//if (m_song_num>0 && !m_playlist[0].file_name.empty())		//�����ǰ·���и������ͱ��浱ǰ·�������·��
	EmplaceCurrentPathToRecent();
	m_sort_mode = sort_mode;
	ChangePath(path, track);
	m_current_position_int = position;
	m_current_position.int2time(m_current_position_int);
	//MusicControl(Command::SEEK);
	EmplaceCurrentPathToRecent();		//�����µ�·�������·��
	
}

void CPlayer::OpenFolder(wstring path)
{
	if (m_loading) return;
	if (path.empty() || (path.back() != L'/' && path.back() != L'\\'))		//����򿪵���·��Ϊ�ջ�ĩβû��б�ܣ�����ĩβ����һ��
		path.append(1, L'\\');
	bool path_exist{ false };
	int track;
	int position;
	if (m_song_num>0) EmplaceCurrentPathToRecent();		//�����ǰ·���и������ͱ��浱ǰ·�������·��
	//���򿪵�·���Ƿ��Ѿ����������·����
	for (const auto& a_path_info : m_recent_path)
	{
		if (path == a_path_info.path)
		{
			path_exist = true;
			track = a_path_info.track;
			position = a_path_info.position;
			m_sort_mode = a_path_info.sort_mode;
			break;
		}
	}
	if (path_exist)			//����򿪵�·���Ѿ����������·����
	{
		ChangePath(path, track);
		m_current_position_int = position;
		m_current_position.int2time(m_current_position_int);
		MusicControl(Command::SEEK);
		EmplaceCurrentPathToRecent();		//����򿪵�·�������·��
		SaveRecentPath();
	}
	else		//����򿪵�·�����µ�·��
	{
		m_sort_mode = SM_FILE;
		ChangePath(path);
		EmplaceCurrentPathToRecent();		//�����µ�·�������·��
		SaveRecentPath();
	}
}

void CPlayer::OpenFiles(const vector<wstring>& files, bool play)
{
	if (files.empty()) return;
	if (m_loading) return;
	MusicControl(Command::CLOSE);
	if (m_song_num>0) EmplaceCurrentPathToRecent();		//�ȱ��浱ǰ·���Ͳ��Ž��ȵ����·��
	size_t index;
	wstring path;
	index = files[0].find_last_of(L'\\');
	path = files[0].substr(0, index + 1);		//��ȡ·��
	if (path != m_path)		//����򿪵��ļ����µ�·���У�����������б�������ԭ���б������
	{
		m_path = path;
		m_playlist.clear();
		m_current_position_int = 0;
		m_current_position = { 0,0,0 };
		m_index = 0;
	}
	//EmplaceCurrentPathToRecent();
	SongInfo song_info;
	for (const auto& file : files)
	{
		index = file.find_last_of(L'\\');
		song_info.file_name = file.substr(index + 1);
		m_playlist.push_back(song_info);	//���ļ������浽�����б�
	}
	IniPlayList(true);
	MusicControl(Command::OPEN);
	MusicControl(Command::SEEK);
	if (play)
		//MusicControl(Command::PLAY);
		PlayTrack(m_song_num - files.size());	//���ļ��󲥷���ӵĵ�1����Ŀ
	//IniLyrics();
	SetTitle();		//�õ�ǰ���ڲ��ŵĸ�������Ϊ���ڱ���
	//SetVolume();
}

void CPlayer::OpenAFile(wstring file)
{
	if (file.empty()) return;
	if (m_loading) return;
	MusicControl(Command::CLOSE);
	if (m_song_num>0) EmplaceCurrentPathToRecent();		//�ȱ��浱ǰ·���Ͳ��Ž��ȵ����·��
	size_t index;
	wstring path;
	index = file.rfind(L'\\');
	path = file.substr(0, index + 1);		//��ȡ·��
	m_path = path;
	m_playlist.clear();
	m_current_position_int = 0;
	m_current_position = { 0,0,0 };
	m_index = 0;
	m_current_file_name = file.substr(index + 1);
	m_song_num = 1;

	//��ȡ��·��������ʽ
	m_sort_mode = SortMode::SM_FILE;
	for (const auto& path_info : m_recent_path)
	{
		if (m_path == path_info.path)
			m_sort_mode = path_info.sort_mode;
	}

	//��ʼ�������б�
	m_current_file_name_tmp = m_current_file_name;
	IniPlayList(false, false);		//������·�����³�ʼ�������б�
}

void CPlayer::SetRepeatMode()
{
	int repeat_mode{ static_cast<int>(m_repeat_mode) };
	repeat_mode++;
	if (repeat_mode > 3)
		repeat_mode = 0;
	m_repeat_mode = static_cast<RepeatMode>(repeat_mode);
	SaveConfig();
}

void CPlayer::SetRepeatMode(RepeatMode repeat_mode)
{
	m_repeat_mode = repeat_mode;
	SaveConfig();
}

RepeatMode CPlayer::GetRepeatMode() const
{
	return m_repeat_mode;
}

bool CPlayer::GetBASSError()
{
	if (m_loading)
		return false;
	int error_code_tmp = BASS_ErrorGetCode();
	if (error_code_tmp && error_code_tmp != m_error_code)
	{
		CString info;
		info.Format(CCommon::LoadText(IDS_BASS_ERROR_LOG_INFO, _T("%d")), error_code_tmp);
		CCommon::WriteLog((theApp.m_module_dir + L"error.log").c_str(), wstring{ info });
	}
	m_error_code = error_code_tmp;
	return true;
}

bool CPlayer::IsError() const
{
	if (m_loading)		//��������б����ڼ��أ��򲻼�����
		return false;
	else
		return (m_error_code != 0 || m_musicStream == 0);
}

void CPlayer::SetTitle() const
{
//#ifdef _DEBUG
//	SetWindowText(theApp.m_pMainWnd->m_hWnd, (m_current_file_name + L" - MusicPlayer2(DEBUGģʽ)").c_str());		//�õ�ǰ���ڲ��ŵĸ�������Ϊ���ڱ���
//#else
//	SetWindowText(theApp.m_pMainWnd->m_hWnd, (m_current_file_name + L" - MusicPlayer2").c_str());		//�õ�ǰ���ڲ��ŵĸ�������Ϊ���ڱ���
//#endif
	SendMessage(theApp.m_pMainWnd->m_hWnd, WM_SET_TITLE, 0, 0);
}

void CPlayer::SaveConfig() const
{
	CIniHelper ini;
	ini.SetPath(theApp.m_config_path);

	//ini.WriteString(L"config", L"path", m_path.c_str());
	//ini.WriteInt(L"config", L"track", m_index);
	ini.WriteInt(L"config", L"volume", m_volume);
	//ini.WriteInt(L"config", L"position", m_current_position_int);
	ini.WriteInt(L"config", L"repeat_mode", static_cast<int>(m_repeat_mode));
	ini.WriteInt(L"config", L"lyric_karaoke_disp", theApp.m_lyric_setting_data.lyric_karaoke_disp);
	ini.WriteString(L"config",L"lyric_path", theApp.m_lyric_setting_data.lyric_path);
	ini.WriteInt(L"config", L"sort_mode", static_cast<int>(m_sort_mode));
	ini.WriteInt(L"config", L"lyric_fuzzy_match", theApp.m_lyric_setting_data.lyric_fuzzy_match);
	ini.WriteString(L"config",L"default_album_file_name", CCommon::StringMerge(theApp.m_app_setting_data.default_album_name, L','));

	//����������趨
	ini.WriteInt(L"equalizer", L"equalizer_enable", m_equ_enable);
	//����ÿ��������ͨ��������
	//if (m_equ_style == 9)
	//{
	//	wchar_t buff[16];
	//	for (int i{}; i < EQU_CH_NUM; i++)
	//	{
	//		swprintf_s(buff, L"channel%d", i + 1);
	//		ini.WriteInt(L"equalizer", buff, m_equalizer_gain[i]);
	//	}
	//}
	//��������趨
	ini.WriteInt(L"reverb", L"reverb_enable", m_reverb_enable);
	ini.WriteInt(L"reverb", L"reverb_mix", m_reverb_mix);
	ini.WriteInt(L"reverb", L"reverb_time", m_reverb_time);
}

void CPlayer::LoadConfig()
{
	CIniHelper ini;
	ini.SetPath(theApp.m_config_path);

	//ini.GetString(L"config", L"path", L".\\songs\\");
	//m_path = buff;
	if (!m_path.empty() && m_path.back() != L'/' && m_path.back() != L'\\')		//�����ȡ������·��ĩβû��б�ܣ�����ĩβ����һ��
		m_path.append(1, L'\\');
	//m_index =ini.GetInt(L"config", L"track", 0);
	m_volume =ini.GetInt(L"config", L"volume", 60);
	//m_current_position_int =ini.GetInt(L"config", L"position", 0);
	//m_current_position.int2time(m_current_position_int);
	m_repeat_mode = static_cast<RepeatMode>(ini.GetInt(L"config", L"repeat_mode", 0));
	theApp.m_lyric_setting_data.lyric_path = ini.GetString(L"config", L"lyric_path", L".\\lyrics\\");
	if (!theApp.m_lyric_setting_data.lyric_path.empty() && theApp.m_lyric_setting_data.lyric_path.back() != L'/' && theApp.m_lyric_setting_data.lyric_path.back() != L'\\')
		theApp.m_lyric_setting_data.lyric_path.append(1, L'\\');
	theApp.m_lyric_setting_data.lyric_karaoke_disp =ini.GetBool(L"config", L"lyric_karaoke_disp", 1);
	m_sort_mode = static_cast<SortMode>(ini.GetInt(L"config", L"sort_mode", 0));
	theApp.m_lyric_setting_data.lyric_fuzzy_match =ini.GetBool(L"config", L"lyric_fuzzy_match", 1);
	wstring default_album_name = ini.GetString(L"config", L"default_album_file_name", L"cover");
	CCommon::StringSplit(default_album_name, L',', theApp.m_app_setting_data.default_album_name);

	//��ȡ�������趨
	m_equ_enable =ini.GetBool(L"equalizer", L"equalizer_enable", 0);
	m_equ_style =ini.GetInt(L"equalizer", L"equalizer_style", 0);	//��ȡ������Ԥ��
	if (m_equ_style == 9)		//���������Ԥ��Ϊ���Զ��塱
	{
		//��ȡÿ��������ͨ��������
		for (int i{}; i < EQU_CH_NUM; i++)
		{
			wchar_t buff[16];
			swprintf_s(buff, L"channel%d", i + 1);
			m_equalizer_gain[i] =ini.GetInt(L"equalizer", buff, 0);
		}
	}
	else if (m_equ_style >= 0 && m_equ_style < 9)		//���򣬸��ݾ�����Ԥ������ÿ��ͨ��������
	{
		for (int i{}; i < EQU_CH_NUM; i++)
		{
			m_equalizer_gain[i] = EQU_STYLE_TABLE[m_equ_style][i];
		}
	}
	//��ȡ�����趨
	m_reverb_enable =ini.GetBool(L"reverb", L"reverb_enable", 0);
	m_reverb_mix =ini.GetInt(L"reverb", L"reverb_mix", 45);		//����ǿ��Ĭ��Ϊ50
	m_reverb_time =ini.GetInt(L"reverb", L"reverb_time", 100);	//����ʱ��Ĭ��Ϊ1s
}

void CPlayer::ExplorePath(int track) const
{
	if (m_song_num > 0)
	{
		CString str;
		if (track < 0)		//trackС��0������Դ��������ѡ�е�ǰ���ŵ��ļ�
			str.Format(_T("/select,\"%s%s\""), m_path.c_str(), m_current_file_name.c_str());
		else if (track < m_song_num)		//trackΪ�����б��е�һ����ţ�����Դ��������ѡ��ָ�����ļ�
			str.Format(_T("/select,\"%s%s\""), m_path.c_str(), m_playlist[track].file_name.c_str());
		else								//track���������б����ļ�������������Դ��������ѡ���κ��ļ�
			str = m_path.c_str();
		ShellExecute(NULL, _T("open"),_T("explorer"), str, NULL, SW_SHOWNORMAL);
	}
}

void CPlayer::ExploreLyric() const
{
	if (!m_Lyrics.IsEmpty())
	{
		CString str;
		str.Format(_T("/select,\"%s\""), m_Lyrics.GetPathName().c_str());
		ShellExecute(NULL, _T("open"), _T("explorer"), str, NULL, SW_SHOWNORMAL);
	}
}


Time CPlayer::GetAllSongLength(int track) const
{
	if (track >= 0 && track < m_playlist.size())
		return m_playlist[track].lengh;
	else
		return Time();
}

void CPlayer::DeleteAlbumCover()
{
	if (!m_inner_cover)
	{
		if (CCommon::DeleteAFile(theApp.m_pMainWnd->GetSafeHwnd(), m_album_cover_path.c_str()) == 0)
			m_album_cover.Destroy();
	}
}

void CPlayer::ReloadPlaylist()
{
	if (m_loading) return;
	MusicControl(Command::CLOSE);
	m_playlist.clear();		//��ղ����б�
	//wstring current_file_name = m_current_file_name;	//���浱ǰ���ŵ���Ŀ���ļ���
	m_current_file_name_tmp = m_current_file_name;	//���浱ǰ���ŵ���Ŀ���ļ����������ڲ����б��ʼ������ʱȷ�����ŵĻ���֮ǰ���ŵ���Ŀ
	//��ʼ�������б�
	IniPlayList(false, true);		//������·�����³�ʼ�������б�


	////�������벥���б�󣬲������ڲ�����Ŀ�����
	//for (int i{}; i < m_playlist.size(); i++)
	//{
	//	if (current_file_name == m_playlist[i].file_name)
	//	{
	//		m_index = i;
	//		m_current_file_name = current_file_name;
	//		break;
	//	}
	//}

	//SetTitle();
	//MusicControl(Command::OPEN);
	//MusicControl(Command::SEEK);
	//SaveConfig();

}

void CPlayer::RemoveSong(int index)
{
	if (m_loading) return;
	if (index >= 0 && index < m_song_num)
	{
		m_playlist.erase(m_playlist.begin() + index);
		m_song_num--;
		if (index == m_index)		//���Ҫɾ������Ŀ�����ڲ��ŵ���Ŀ�����²��ŵ�ǰ��Ŀ
		{
			if(m_song_num>0)
				PlayTrack(m_index);
		}
		else if (index < m_index)	//���Ҫɾ������Ŀ�����ڲ��ŵ���Ŀ֮ǰ�������ڲ��ŵ���Ŀ��ż�1
		{
			m_index--;
		}
		if (m_song_num == 0)
			m_playlist.push_back(SongInfo());
	}
}

void CPlayer::RemoveSongs(vector<int> indexes)
{
	int size = indexes.size();
	for (int i{}; i<size; i++)
	{
		RemoveSong(indexes[i]);
		if (i <= size - 2 && indexes[i + 1] > indexes[i])
		{
			for (int j{ i + 1 }; j < size; j++)
				indexes[j]--;
		}
	}
}

void CPlayer::ClearPlaylist()
{
	if (m_loading) return;
	m_playlist.clear();
	m_song_num = 0;
	MusicControl(Command::STOP);
}

void CPlayer::SeekTo(int position)
{
	if (position > m_song_length_int)
		position = m_song_length_int;
	m_current_position_int = position;
	m_current_position.int2time(position);
	if (m_playlist[m_index].is_cue)
	{
		position += m_playlist[m_index].start_pos.time2int();
	}
	double pos_sec = static_cast<double>(position) / 1000.0;
	QWORD pos_bytes;
	pos_bytes = BASS_ChannelSeconds2Bytes(m_musicStream, pos_sec);
	BASS_ChannelSetPosition(m_musicStream, pos_bytes, BASS_POS_BYTE);
	m_midi_lyric.clear();
	GetMidiPosition();
}

void CPlayer::SeekTo(double position)
{
	int pos = static_cast<int>(m_song_length_int*position);
	SeekTo(pos);
}

void CPlayer::SeekTo(HSTREAM hStream, int position)
{
	double pos_sec = static_cast<double>(position) / 1000.0;
	QWORD pos_bytes;
	pos_bytes = BASS_ChannelSeconds2Bytes(hStream, pos_sec);
	BASS_ChannelSetPosition(hStream, pos_bytes, BASS_POS_BYTE);
}

void CPlayer::ClearLyric()
{
	m_Lyrics = CLyrics{};
	m_playlist[m_index].lyric_file.clear();
}

wstring CPlayer::GetTimeString() const
{
	wchar_t buff[16];
	swprintf_s(buff, L"%d:%.2d/%d:%.2d", m_current_position.min, m_current_position.sec, m_song_length.min, m_song_length.sec);
	return wstring(buff);
}

wstring CPlayer::GetPlayingState() const
{
	if (m_error_code != 0)
		return CCommon::LoadText(IDS_PLAY_ERROR).GetString();
	switch (m_playing)
	{
	case 0: return CCommon::LoadText(IDS_STOPED).GetString();
	case 1: return CCommon::LoadText(IDS_PAUSED).GetString();
	case 2: return CCommon::LoadText(IDS_NOW_PLAYING).GetString();
	}
	return wstring();
}

const SongInfo & CPlayer::GetCurrentSongInfo() const
{
	// TODO: �ڴ˴����� return ���
	if (m_index >= 0 && m_index < m_playlist.size())
		return m_playlist[m_index];
	else return m_no_use;
}

void CPlayer::SetRelatedSongID(wstring song_id)
{
	if (m_index >= 0 && m_index < m_playlist.size())
	{
		m_playlist[m_index].song_id = song_id;
		if(!m_playlist[m_index].is_cue)
			theApp.m_song_data[m_path + m_playlist[m_index].file_name] = m_playlist[m_index];
	}
}

void CPlayer::SetRelatedSongID(int index, wstring song_id)
{
	if (index >= 0 && index < m_playlist.size())
	{
		m_playlist[index].song_id = song_id;
		if (!m_playlist[index].is_cue)
			theApp.m_song_data[m_path + m_playlist[index].file_name] = m_playlist[index];
	}
}

void CPlayer::ReIniBASS(bool replay)
{
	int playing = m_playing;
	UnInitBASS();
	IniBASS();
	MusicControl(Command::OPEN);
	MusicControl(Command::SEEK);
	if (replay && playing == 2)
	{
		MusicControl(Command::PLAY);
	}
	else
	{
		m_playing = 0;
	}
}

void CPlayer::SortPlaylist(bool change_index)
{
	if (m_loading) return;
	int track_number = m_playlist[m_index].track;
	switch (m_sort_mode)
	{
	case SM_FILE: std::sort(m_playlist.begin(), m_playlist.end(), SongInfo::ByFileName);
		break;
	case SM_TITLE: std::sort(m_playlist.begin(), m_playlist.end(), SongInfo::ByTitle);
		break;
	case SM_ARTIST: std::sort(m_playlist.begin(), m_playlist.end(), SongInfo::ByArtist);
		break;
	case SM_ALBUM: std::sort(m_playlist.begin(), m_playlist.end(), SongInfo::ByAlbum);
		break;
	case SM_TRACK: std::sort(m_playlist.begin(), m_playlist.end(), SongInfo::ByTrack);
		break;
	default:
		break;
	}

	if (change_index)
	{
		//�����б�����󣬲������ڲ�����Ŀ�����
		if (!m_playlist[m_index].is_cue)
		{
			for (int i{}; i < m_playlist.size(); i++)
			{
				if (m_current_file_name == m_playlist[i].file_name)
				{
					m_index = i;
					break;
				}
			}
		}
		else
		{
			for (int i{}; i < m_playlist.size(); i++)
			{
				if (track_number == m_playlist[i].track)
				{
					m_index = i;
					break;
				}
			}
		}
	}
}


void CPlayer::SaveRecentPath() const
{
	// �򿪻����½��ļ�
	CFile file;
	BOOL bRet = file.Open(theApp.m_recent_path_dat_path.c_str(),
		CFile::modeCreate | CFile::modeWrite);
	if (!bRet)		//���ļ�ʧ��
	{
		return;
	}
	// ����CArchive����
	CArchive ar(&file, CArchive::store);
	// д����
	ar << m_recent_path.size();		//д��m_recent_path�����Ĵ�С
	for (auto& path_info : m_recent_path)
	{
		ar << CString(path_info.path.c_str())
			<< path_info.track
			<< path_info.position
			<< static_cast<int>(path_info.sort_mode)
			<< path_info.track_num
			<< path_info.total_time;
	}
	// �ر�CArchive����
	ar.Close();
	// �ر��ļ�
	file.Close();

}

void CPlayer::OnExit()
{
	SaveConfig();
	//�˳�ʱ������󲥷ŵ���Ŀ��λ��
	if (!m_recent_path.empty() && m_song_num>0 && !m_playlist[0].file_name.empty())
	{
		m_recent_path[0].track = m_index;
		m_recent_path[0].position = m_current_position_int;
	}
	SaveRecentPath();
}

void CPlayer::LoadRecentPath()
{
	// ���ļ�
	CFile file;
	BOOL bRet = file.Open(theApp.m_recent_path_dat_path.c_str(), CFile::modeRead);
	if (!bRet)		//�ļ�������
	{
		m_path = L".\\songs\\";		//Ĭ�ϵ�·��
		return;
	}
	// ����CArchive����
	CArchive ar(&file, CArchive::load);
	// ������
	size_t size{};
	PathInfo path_info;
	CString temp;
	int sort_mode;
	try
	{
		ar >> size;		//��ȡӳ�������ĳ���
		for (size_t i{}; i < size; i++)
		{
			ar >> temp;
			path_info.path = temp;
			ar >> path_info.track;
			ar >> path_info.position;
			ar >> sort_mode;
			path_info.sort_mode = static_cast<SortMode>(sort_mode);
			ar >> path_info.track_num;
			ar >> path_info.total_time;
			if (path_info.path.empty() || path_info.path.size() < 2) continue;		//���·��Ϊ�ջ�·��̫�̣��ͺ�����
			if (path_info.path.back() != L'/' && path_info.path.back() != L'\\')	//�����ȡ����·��ĩβû��б�ܣ�����ĩβ����һ��
				path_info.path.push_back(L'\\');
			m_recent_path.push_back(path_info);
		}
	}
	catch (CArchiveException* exception)
	{
		//�������л�ʱ���ֵ��쳣
		CString info;
		info = CCommon::LoadTextFormat(IDS_RECENT_PATH_SERIALIZE_ERROR_LOG, { exception->m_cause });
		CCommon::WriteLog((theApp.m_module_dir + L"error.log").c_str(), wstring{ info });
	}
	// �رն���
	ar.Close();
	// �ر��ļ�
	file.Close();

	//��recent_path�ļ��л�ȡ·�������ŵ�����Ŀ��λ��
	if (!m_recent_path.empty())
	{
		m_path = m_recent_path[0].path;
		m_index = m_recent_path[0].track;
		m_current_position_int = m_recent_path[0].position;
		m_current_position.int2time(m_current_position_int);
	}
	else
	{
		m_path = L".\\songs\\";		//Ĭ�ϵ�·��
	}
}

void CPlayer::EmplaceCurrentPathToRecent()
{
	for (int i{ 0 }; i < m_recent_path.size(); i++)
	{
		if (m_path == m_recent_path[i].path)
			m_recent_path.erase(m_recent_path.begin() + i);		//�����ǰ·���Ѿ������·���У��Ͱ������·����ɾ��
	}
	if (m_song_num == 0 || m_playlist[0].file_name.empty()) return;		//�����ǰ·����û���ļ����Ͳ�����
	PathInfo path_info;
	path_info.path = m_path;
	path_info.track = m_index;
	path_info.position = m_current_position_int;
	path_info.sort_mode = m_sort_mode;
	path_info.track_num = m_song_num;
	path_info.total_time = m_total_time;
	if (m_song_num > 0)
		m_recent_path.push_front(path_info);		//��ǰ·�����뵽m_recent_path��ǰ��
}


void CPlayer::SetFXHandle()
{
	if (m_musicStream == 0) return;
	//if (!m_equ_enable) return;
	//����ÿ��������ͨ���ľ��
	for (int i{}; i < EQU_CH_NUM; i++)
	{
		m_equ_handle[i] = BASS_ChannelSetFX(m_musicStream, BASS_FX_DX8_PARAMEQ, 1);
	}
	//���û���ľ��
	m_reverb_handle = BASS_ChannelSetFX(m_musicStream, BASS_FX_DX8_REVERB, 1);
}

void CPlayer::RemoveFXHandle()
{
	if (m_musicStream == 0) return;
	//�Ƴ�ÿ��������ͨ���ľ��
	for (int i{}; i < EQU_CH_NUM; i++)
	{
		if (m_equ_handle[i] != 0)
		{
			BASS_ChannelRemoveFX(m_musicStream, m_equ_handle[i]);
			m_equ_handle[i] = 0;
		}
	}
	//�Ƴ�����ľ��
	if (m_reverb_handle != 0)
	{
		BASS_ChannelRemoveFX(m_musicStream, m_reverb_handle);
		m_reverb_handle = 0;
	}
}

void CPlayer::ApplyEqualizer(int channel, int gain)
{
	if (channel < 0 || channel >= EQU_CH_NUM) return;
	//if (!m_equ_enable) return;
	if (gain < -15) gain = -15;
	if (gain > 15) gain = 15;
	BASS_DX8_PARAMEQ parameq;
	parameq.fBandwidth = 30;
	parameq.fCenter = FREQ_TABLE[channel];
	parameq.fGain = static_cast<float>(gain);
	BASS_FXSetParameters(m_equ_handle[channel], &parameq);
}

void CPlayer::SetEqualizer(int channel, int gain)
{
	if (channel < 0 || channel >= EQU_CH_NUM) return;
	m_equalizer_gain[channel] = gain;
	ApplyEqualizer(channel, gain);
}

int CPlayer::GeEqualizer(int channel)
{
	if (channel < 0 || channel >= EQU_CH_NUM) return 0;
	//BASS_DX8_PARAMEQ parameq;
	//int rtn;
	//rtn = BASS_FXGetParameters(m_equ_handle[channel], &parameq);
	//return static_cast<int>(parameq.fGain);
	return m_equalizer_gain[channel];
}

void CPlayer::SetAllEqualizer()
{
	for (int i{}; i < EQU_CH_NUM; i++)
	{
		ApplyEqualizer(i, m_equalizer_gain[i]);
	}
}

void CPlayer::ClearAllEqulizer()
{
	for (int i{}; i < EQU_CH_NUM; i++)
	{
		ApplyEqualizer(i, 0);
	}
}

void CPlayer::EnableEqualizer(bool enable)
{
	if (enable)
		SetAllEqualizer();
	else
		ClearAllEqulizer();
	m_equ_enable = enable;
}

void CPlayer::SetReverb(int mix, int time)
{
	if (mix < 0) mix = 0;
	if (mix > 100) mix = 100;
	if (time < 1) time = 1;
	if (time > 300) time = 300;
	m_reverb_mix = mix;
	m_reverb_time = time;
	BASS_DX8_REVERB parareverb;
	parareverb.fInGain = 0;
	//parareverb.fReverbMix = static_cast<float>(mix) / 100 * 96 - 96;
	parareverb.fReverbMix = static_cast<float>(std::pow(static_cast<double>(mix) / 100, 0.1) * 96 - 96);
	parareverb.fReverbTime = static_cast<float>(time * 10);
	parareverb.fHighFreqRTRatio = 0.001f;
	BASS_FXSetParameters(m_reverb_handle, &parareverb);
}

void CPlayer::ClearReverb()
{
	BASS_DX8_REVERB parareverb;
	parareverb.fInGain = 0;
	parareverb.fReverbMix = -96;
	parareverb.fReverbTime = 0.001f;
	parareverb.fHighFreqRTRatio = 0.001f;
	BASS_FXSetParameters(m_reverb_handle, &parareverb);
}

void CPlayer::EnableReverb(bool enable)
{
	if (enable)
		SetReverb(m_reverb_mix, m_reverb_time);
	else
		ClearReverb();
	m_reverb_enable = enable;
}


void CPlayer::ConnotPlayWarning() const
{
	if (m_is_midi && m_sfont.font == 0)
		PostMessage(theApp.m_pMainWnd->GetSafeHwnd(), WM_CONNOT_PLAY_WARNING, 0, 0);
}

void CPlayer::SearchAlbumCover()
{
	static wstring last_file_path;
	if (last_file_path != m_path + m_current_file_name)		//��ֹͬһ���ļ���λ�ȡר������
	{
		m_album_cover.Destroy();
		if (!theApp.m_app_setting_data.use_out_image || theApp.m_app_setting_data.use_inner_image_first)
		{
			//���ļ���ȡר������
			CAudioTag audio_tag(m_musicStream, m_path + m_current_file_name, m_playlist[m_index]);
			m_album_cover_path = audio_tag.GetAlbumCover(m_album_cover_type);
			m_album_cover.Load(m_album_cover_path.c_str());
		}
		m_inner_cover = !m_album_cover.IsNull();

		if (/*theApp.m_app_setting_data.use_out_image && */m_album_cover.IsNull())
		{
			//��ȡ����ר������ʱ����ʹ���ⲿͼƬ��Ϊ����
			SearchOutAlbumCover();
		}
		//AlbumCoverGaussBlur();
	}
	last_file_path = m_path + m_current_file_name;
}

void CPlayer::AlbumCoverGaussBlur()
{
	if (!theApp.m_app_setting_data.background_gauss_blur || !theApp.m_app_setting_data.album_cover_as_background)
		return;
	if (m_album_cover.IsNull())
	{
		m_album_cover_blur.Destroy();
	}
	else
	{
		CImage image_tmp;
		CSize image_size(m_album_cover.GetWidth(), m_album_cover.GetHeight());
		//��ͼƬ��С�Լ�С��˹ģ���ļ�����
		CCommon::SizeZoom(image_size, 300);		//ͼƬ��С���������ţ�ʹ���ߵ���300
		if (!CDrawCommon::BitmapStretch(&m_album_cover, &image_tmp, image_size))		//����ͼƬ
			return;
#ifdef _DEBUG
		image_tmp.Save(_T("..\\Debug\\image_tmp.bmp"), Gdiplus::ImageFormatBMP);
#endif // _DEBUG

		//ִ�и�˹ģ��
		CGaussBlur gauss_blur;
		gauss_blur.SetSigma(static_cast<double>(theApp.m_app_setting_data.gauss_blur_radius) / 10);		//���ø�˹ģ���뾶
		gauss_blur.DoGaussBlur(image_tmp, m_album_cover_blur);
	}
}

void CPlayer::GetMidiPosition()
{
	if (m_is_midi)
	{
		//��ȡmidi���ֵĽ��Ȳ�ת���ɽ�������������+ (m_midi_info.ppqn / 4)��Ŀ����������ʾ�Ľ��Ĳ�׼ȷ�����⣩
		m_midi_info.midi_position = static_cast<int>((BASS_ChannelGetPosition(m_musicStream, BASS_POS_MIDI_TICK) + (m_midi_info.ppqn / 4)) / m_midi_info.ppqn);
	}
}

void CPlayer::AcquireSongInfo(HSTREAM hStream, const wstring& file_path, SongInfo & song_info)
{
	//��ȡ����
	song_info.lengh = GetBASSSongLength(hStream);
	//��ȡ������
	float bitrate{};
	BASS_ChannelGetAttribute(hStream, BASS_ATTRIB_BITRATE, &bitrate);
	song_info.bitrate = static_cast<int>(bitrate + 0.5f);
	//��ȡ��Ƶ��ǩ
	CAudioTag audio_tag(hStream, file_path, song_info);
	audio_tag.GetAudioTag(theApp.m_general_setting_data.id3v2_first);
	//��ȡmidi���ֵı���
	if (m_bass_midi_lib.IsSuccessed() && audio_tag.GetAudioType() == AU_MIDI)
	{
		BASS_MIDI_MARK mark;
		if (m_bass_midi_lib.BASS_MIDI_StreamGetMark(hStream, BASS_MIDI_MARK_TRACK, 0, &mark) && !mark.track)
		{
			song_info.title = CCommon::StrToUnicode(mark.text);
			song_info.info_acquired = true;
		}
	}
	CFilePathHelper c_file_path(file_path);
	song_info.file_name = c_file_path.GetFileName();
	//���������Ϣ
	theApp.m_song_data[file_path] = song_info;
}

void CPlayer::SearchOutAlbumCover()
{
	m_album_cover_path = GetRelatedAlbumCover(m_path + m_current_file_name, GetCurrentSongInfo());
	if (!m_album_cover.IsNull())
		m_album_cover.Destroy();
	m_album_cover.Load(m_album_cover_path.c_str());
}

wstring CPlayer::GetRelatedAlbumCover(const wstring& file_path, const SongInfo& song_info)
{
	vector<wstring> files;
	wstring file_name;
	//�����ļ��͸�����һ�µ�ͼƬ�ļ�
	CFilePathHelper c_file_path(file_path);
	//file_name = m_path + c_file_name.GetFileNameWithoutExtension() + L".*";
	c_file_path.ReplaceFileExtension(L"*");
	wstring dir{ c_file_path.GetDir() };
	CCommon::GetImageFiles(c_file_path.GetFilePath(), files);
	if (files.empty() && !song_info.album.empty())
	{
		//û���ҵ��͸�����һ�µ�ͼƬ�ļ���������ļ���Ϊ����Ƭ�������ļ�
		wstring album_name{ song_info.album };
		CCommon::FileNameNormalize(album_name);
		file_name = dir + album_name + L".*";
		CCommon::GetImageFiles(file_name, files);
	}
	//if (files.empty() && !theApp.m_app_setting_data.default_album_name.empty())
	//{
	//	//û���ҵ���Ƭ��Ϊ�ļ������ļ��������ļ���ΪDEFAULT_ALBUM_NAME���ļ�
	//	file_name = m_path + theApp.m_app_setting_data.default_album_name + L".*";
	//	CCommon::GetImageFiles(file_name, files);
	//}
	//û���ҵ���Ƭ��Ϊ�ļ������ļ��������ļ���Ϊ���õ�ר�����������ļ�
	if (theApp.m_app_setting_data.use_out_image)
	{
		for (const auto& album_name : theApp.m_app_setting_data.default_album_name)
		{
			if (!files.empty())
				break;
			if (!album_name.empty())
			{
				file_name = dir + album_name + L".*";
				CCommon::GetImageFiles(file_name, files);
			}
		}
	}
	//if (files.empty())
	//{
	//	//û���ҵ��ļ���ΪDEFAULT_ALBUM_NAME���ļ���������Ϊ��Folder���ļ���
	//	file_name = m_path + L"Folder" + L".*";
	//	CCommon::GetImageFiles(file_name, files);
	//}
	if (!files.empty())
	{
		//m_album_cover_path = m_path + files[0];
		//if (!m_album_cover.IsNull())
		//	m_album_cover.Destroy();
		//m_album_cover.Load(m_album_cover_path.c_str());
		return wstring(dir + files[0]);
	}
	else
	{
		return wstring();
	}
}
