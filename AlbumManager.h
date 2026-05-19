#pragma once
#include <vector>
#include "Constants.h"
#include "MemoryAccess.h"
#include "Album.h"
#include <windows.h>


class AlbumManager
{
public:
	AlbumManager(IDataAccess& dataAccess);
	static BOOL WINAPI consoleHandler(DWORD signal);
	void executeCommand(CommandType command);
	void printHelp() const;

	using handler_func_t = void (AlbumManager::*)(void);    

private:
	static HANDLE _global_hAppPic;
    int m_nextPictureId{};
    int m_nextUserId{};
    std::string m_currentAlbumName{};
	IDataAccess& m_dataAccess;
	Album m_openAlbum;
	  
	void help();
	// albums management
	void createAlbum();
	void openAlbum();
	void closeAlbum();
	void deleteAlbum();
	void listAlbums();
	void listAlbumsOfUser();

	// Picture management
	void addPictureToAlbum();
	void removePictureFromAlbum();
	void listPicturesInAlbum();
	void showPicture();

	// tags related
	void tagUserInPicture();
	void untagUserInPicture();
	void listUserTags();

	// users management
	void addUser();
	void removeUser();
	void listUsers();
	void userStatistics();

	void topTaggedUser();
	void topTaggedPicture();
	void picturesTaggedUser();
	void exit();

	std::string getInputFromConsole(const std::string& message);
	bool fileExistsOnDisk(const std::string& filename);
	void refreshOpenAlbum();
    bool isCurrentAlbumSet() const;

	void AlbumManager::openPicInSpecificApp(const std::string& appPath, const std::string& picPath);


	static const std::vector<struct CommandGroup> m_prompts;
	static const std::map<CommandType, handler_func_t> m_commands;

};

