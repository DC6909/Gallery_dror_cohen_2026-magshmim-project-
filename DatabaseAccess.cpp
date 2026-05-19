#include "DatabaseAccess.h"
#include "ItemNotFoundException.h"

using std::string;
using std::cout;
using std::endl;
//db system fucnitons
DatabaseAccess::DatabaseAccess(std::string dbFileName):IDataAccess(), _dbFileName(dbFileName)
{

}
DatabaseAccess::~DatabaseAccess()
{
	clear();
}
bool DatabaseAccess::open()
{
    char* errMes = nullptr;
    bool  doesFileExist = _access(_dbFileName.c_str(), 0) == 0;// if the file is exist before
    int res = sqlite3_open(_dbFileName.c_str(), &_db);
    if (!doesFileExist)
    {
        // create all the tables if the file wasnt exist before
        const char* sqlUsers = "CREATE TABLE IF NOT EXISTS USERS ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "NAME TEXT NOT NULL);";
        //linked to user table...
        const char* sqlAlbums = "CREATE TABLE IF NOT EXISTS ALBUMS ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "NAME TEXT NOT NULL, "
            "CREATION_DATE DATETIME DEFAULT CURRENT_TIMESTAMP, "
            "USER_ID INTEGER NOT NULL, "
            "FOREIGN KEY(USER_ID) REFERENCES USERS(ID));";
        //linked to albums table...
        const char* sqlPictures = "CREATE TABLE IF NOT EXISTS PICTURES ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "NAME TEXT NOT NULL, "
            "CREATION_DATE DATETIME DEFAULT CURRENT_TIMESTAMP, "
            "PATH TEXT NOT NULL, "
            "ALBUM_ID INTEGER NOT NULL, "
            "FOREIGN KEY(ALBUM_ID) REFERENCES ALBUMS(ID));";
        //linked to user table and pictures table...
        const char* sqlTags = "CREATE TABLE IF NOT EXISTS TAGS ("
            "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL, "
            "PICTURE_ID INTEGER NOT NULL, "
            "USER_ID INTEGER NOT NULL, "
            "FOREIGN KEY(PICTURE_ID) REFERENCES PICTURES(ID), "
            "FOREIGN KEY(USER_ID) REFERENCES USERS(ID));";


        /// put all the commends in array and use for the run all of them


        const char* sqlCommends[] = { sqlUsers,sqlAlbums,sqlPictures,sqlTags };

        for (const char* cmd : sqlCommends) {
            res = sqlite3_exec(_db, cmd, nullptr, nullptr, &errMes);

            if (res != SQLITE_OK) {
                std::cout << "Error: " << errMes << std::endl;
                sqlite3_free(errMes);
                break;
            }
        }
    }
    const char* sqlEnableFK = "PRAGMA foreign_keys = ON;";// the gimini say that i need to add this beacuse in the defaluse the sql not enforce foreign key so if i not add this i wiil can delete user whit out delet is alcum first and not give me a error  
    res = sqlite3_exec(_db, sqlEnableFK, nullptr, nullptr, &errMes);
    if (res != SQLITE_OK) {
        std::cout << "SQL Error: " << errMes << std::endl;
        sqlite3_free(errMes);
    }
	return res == SQLITE_OK;
};
void DatabaseAccess::close()
{
	sqlite3_close(_db);
	_db = nullptr;
}
void DatabaseAccess::clear()
{
	close();
}


// Album related
const std::list<Album> DatabaseAccess::getAlbums()
{
    std::list<Album> albums;

    sqlite3_stmt* stmtAlbums;
    string sqlCmd = "SELECT * FROM ALBUMS;";
    
    if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmtAlbums, nullptr) == SQLITE_OK)
    {
        // need to step oer all the albums
        while (sqlite3_step(stmtAlbums) == SQLITE_ROW)
        {
            int userId = sqlite3_column_int(stmtAlbums, 3);
            int albumId = sqlite3_column_int(stmtAlbums, 0);
            const unsigned char* name = sqlite3_column_text(stmtAlbums, 1);
            const unsigned char* creationTime = sqlite3_column_text(stmtAlbums, 2);


            Album album(userId, reinterpret_cast<const char*>(name),
                reinterpret_cast<const char*>(creationTime));

            sqlite3_stmt* stmtPicture;
            sqlCmd = "SELECT * FROM PICTURES WHERE ALBUM_ID =" + std::to_string(albumId) + ";";
            if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmtPicture, nullptr) == SQLITE_OK)
            {
                //need also add all the picture on the album for eace album and for this need to anther sql querie
                while (sqlite3_step(stmtPicture) == SQLITE_ROW)
                {
                    int picId = sqlite3_column_int(stmtPicture, 0);
                    const unsigned char* name = sqlite3_column_text(stmtPicture, 1);
                    const unsigned char* creationTime = sqlite3_column_text(stmtPicture, 2);
                    const unsigned char* path = sqlite3_column_text(stmtPicture, 3);

                    
                    
                    Picture pic(picId,
                        reinterpret_cast<const char*>(name),
                        reinterpret_cast<const char*>(path),
                        reinterpret_cast<const char*>(creationTime));
                    // we need also add all the tags on user to the picture
                    sqlite3_stmt* stmtTags;
                    std::string sqlTags = "SELECT USER_ID FROM TAGS WHERE PICTURE_ID = " + std::to_string(picId) + ";";

                    if (sqlite3_prepare_v2(_db, sqlTags.c_str(), -1, &stmtTags, nullptr) == SQLITE_OK) {
                        while (sqlite3_step(stmtTags) == SQLITE_ROW) {
                            int taggedUserId = sqlite3_column_int(stmtTags, 0);

                            pic.tagUser(taggedUserId);// add the user id for the set of tagged users ids
                        }
                        sqlite3_finalize(stmtTags);
                    }

                    album.addPicture(pic);
                }
                sqlite3_finalize(stmtPicture);
            }
            albums.push_back(album);
        }
    }
    sqlite3_finalize(stmtAlbums);
    return albums;
}
const std::list<std::string> DatabaseAccess::getAlbumsNamesOfUser(const User& user)
{
    std::list<std::string> albumNames;
    sqlite3_stmt* stmt;// use stmt and not exec becuase this more easy way to get data back from the server 

    std::string sqlCmd = "SELECT NAME FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + ";";// we get all the colum name of the albums of user whit id...

    int res = sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmt, nullptr);//ask...

    if (res != SQLITE_OK) {
        std::cerr << "SQL Error in getAlbumsNamesOfUser: " << sqlite3_errmsg(_db) << std::endl;
        return albumNames;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {// evary time this fucntion call she move the next row and return if i this a row or no SQLITE_ROW|sQITE_DONE...
        const unsigned char* name = sqlite3_column_text(stmt, 0);// get the data from colum number 0(we ask only one so its the name...) 
        if (name) {
            albumNames.push_back(std::string(reinterpret_cast<const char*>(name)));
        }
    }

    sqlite3_finalize(stmt);// close the stmt 

    return albumNames;
}
const std::list<Album> DatabaseAccess::getAlbumsOfUser(const User& user)
{ 
    std::list<Album> albums;

    sqlite3_stmt* stmtAlbums;
    string sqlCmd = "SELECT * FORM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + ";";

    if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmtAlbums, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmtAlbums) == SQLITE_ROW)
        {
            int userId = sqlite3_column_int(stmtAlbums, 3);
            int albumId = sqlite3_column_int(stmtAlbums, 0);
            const unsigned char* name = sqlite3_column_text(stmtAlbums, 1);
            const unsigned char* creationTime = sqlite3_column_text(stmtAlbums, 2);


            Album album(userId, reinterpret_cast<const char*>(name),
                reinterpret_cast<const char*>(creationTime));

            sqlite3_stmt* stmtPicture;
            sqlCmd = "SELECT * FORM PICTURES WHERE ABLUM_ID =" + std::to_string(albumId) + ";";
            if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmtPicture, nullptr) == SQLITE_OK)
            {
                while (sqlite3_step(stmtPicture) == SQLITE_ROW)
                {
                    int picId = sqlite3_column_int(stmtPicture, 0);
                    const unsigned char* name = sqlite3_column_text(stmtPicture, 1);
                    const unsigned char* creationTime = sqlite3_column_text(stmtPicture, 2);
                    const unsigned char* path = sqlite3_column_text(stmtPicture, 2);
                    int albumId = sqlite3_column_int(stmtPicture, 3);



                    Picture pic(picId,
                        reinterpret_cast<const char*>(name),
                        reinterpret_cast<const char*>(path),
                        reinterpret_cast<const char*>(creationTime));
                    album.addPicture(pic);
                }
                sqlite3_finalize(stmtPicture);
            }
            albums.push_back(album);
        }
    }
    sqlite3_finalize(stmtAlbums);
    return albums;
}

void DatabaseAccess::createAlbum(const Album& album)
{
    sqlite3_stmt* stmt;

    std::string sql = "INSERT INTO ALBUMS (NAME, CREATION_DATE, USER_ID) VALUES ('" // the id of the albums is create on the db automatlicy so we dont need to insert it...
        + album.getName() + "', '"
        + album.getCreationDate() + "', "
        + std::to_string(album.getOwnerId()) + ");";

    if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            sqlite3_finalize(stmt);// like if thear are the ablum whit the same name
            throw std::runtime_error("Failed to create album in DB");
        }
    }
    else
    {
        throw std::runtime_error("Failed to prepare createAlbum statement");
    }

    sqlite3_finalize(stmt);
}

void DatabaseAccess::deleteAlbum(const std::string& albumName, int userId) 
{
    char* errMsg = nullptr;
    removeAllPictureFromAlbum(albumName);

    string sqlCmd = "DELETE FROM ALBUMS WHERE NAME = '" + albumName + "';";

    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMsg);
    if (res != SQLITE_OK) {
        std::cout << "SQL Error in deleteUser: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

}

bool DatabaseAccess::doesAlbumExists(const std::string& albumName, int userId) 
{
    sqlite3_stmt* stmt;
    
    string sqlCmd = "SELECT NAME FROM ALBUMS WHERE USER_ID = " + std::to_string(userId) +
        " AND NAME = '" + albumName + "';";
    if (sqlite3_prepare(_db, sqlCmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if(sqlite3_step(stmt) == SQLITE_ROW)
        {
            return true;
        }
    }
    sqlite3_finalize(stmt);

    return false;
}

Album DatabaseAccess::openAlbum(const std::string& albumName)
{

    sqlite3_stmt* stmtAlbums;


    string sqlCmd = "SELECT * FROM ALBUMS WHERE NAME = '" + albumName + "';";
    if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmtAlbums, nullptr) == SQLITE_OK)
    {
        if(sqlite3_step(stmtAlbums) == SQLITE_ROW)
        {
            int userId = sqlite3_column_int(stmtAlbums, 3);
            int albumId = sqlite3_column_int(stmtAlbums, 0);
            const unsigned char* name = sqlite3_column_text(stmtAlbums, 1);
            const unsigned char* creationTime = sqlite3_column_text(stmtAlbums, 2);


            Album album(userId, reinterpret_cast<const char*>(name),
                reinterpret_cast<const char*>(creationTime));

            sqlite3_stmt* stmtPicture;
            sqlCmd = "SELECT * FROM PICTURES WHERE ALBUM_ID =" + std::to_string(albumId) + ";";
            if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmtPicture, nullptr) == SQLITE_OK)
            {
                while (sqlite3_step(stmtPicture) == SQLITE_ROW)
                {
                    int picId = sqlite3_column_int(stmtPicture, 0);
                    const unsigned char* name = sqlite3_column_text(stmtPicture, 1);
                    const unsigned char* creationTime = sqlite3_column_text(stmtPicture, 2);
                    const unsigned char* path = sqlite3_column_text(stmtPicture, 3);

                    Picture pic(picId,
                        reinterpret_cast<const char*>(name),
                        reinterpret_cast<const char*>(path),
                        reinterpret_cast<const char*>(creationTime));
                    // we need also add all the tags on user to the picture
                    sqlite3_stmt* stmtTags;
                    std::string sqlTags = "SELECT USER_ID FROM TAGS WHERE PICTURE_ID = " + std::to_string(picId) + ";";

                    if (sqlite3_prepare_v2(_db, sqlTags.c_str(), -1, &stmtTags, nullptr) == SQLITE_OK) {
                        while (sqlite3_step(stmtTags) == SQLITE_ROW) {
                            int taggedUserId = sqlite3_column_int(stmtTags, 0);

                            pic.tagUser(taggedUserId);// add the user id for the set of tagged users ids
                        }
                        sqlite3_finalize(stmtTags);
                    }
                    album.addPicture(pic);
                }
                sqlite3_finalize(stmtPicture);
            }
            sqlite3_finalize(stmtAlbums);
            return album;
        }
    }
    sqlite3_finalize(stmtAlbums);
    throw ItemNotFoundException("Album", albumName);
}

void DatabaseAccess::closeAlbum(Album& pAlbum){} // dont need because we store any way the changes on the db

void DatabaseAccess::printAlbums() 
{
    std::list<Album> albums = getAlbums();
    for (auto iAlbum = albums.begin(); iAlbum != albums.end(); iAlbum++)
    {
        std::cout << *iAlbum;
    }
}

// Picture related
int DatabaseAccess::getLastPictureId()
{
    int lastId = 0;
    sqlite3_stmt* stmt;

    std::string sql = "SELECT MAX(ID) FROM PICTURES;";

    if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            lastId = sqlite3_column_int(stmt, 0);
        }
    }
    return lastId;
}
void DatabaseAccess::addPictureToAlbumByName(const std::string& albumName, const Picture& picture)
{
    sqlite3_stmt* stmt;
    int albumId = -1;
    char* errMessage = nullptr;


    // we need first found the id of the album
    std::string sqlGetId = "SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "';";
   
    
    if (sqlite3_prepare_v2(_db, sqlGetId.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            albumId = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);

    // if the album not exist we not have a point to countiu...
    if (albumId == -1) {
        throw ItemNotFoundException("Album", albumName);
    }
    //the secend part is to add the picture by "insert"
    std::string sqlInsert = "INSERT INTO PICTURES (NAME, CREATION_DATE, PATH, ALBUM_ID) VALUES ('"
        + picture.getName() + "', '"
        + picture.getCreationDate() + "', '"
        + picture.getPath() + "', "
        + std::to_string(albumId) + ");";

    int res = sqlite3_exec(_db, sqlInsert.c_str(), nullptr, nullptr, &errMessage);

    if (res != SQLITE_OK) {
        std::string err = errMessage;
        sqlite3_free(errMessage);
        throw std::runtime_error("falide to add pic to db: " + err);
    }
}

void DatabaseAccess::removePictureFromAlbumByName(const std::string& albumName, const std::string& pictureName)
{
    char* errMes = nullptr;


    std::string sqlCmd =
        "DELETE FROM PICTURES WHERE NAME = '" + pictureName + "' AND ALBUM_ID = ("
        "SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "');";

    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMes);

    if (res != SQLITE_OK) {
        std::cout << "SQL Error in removePictureFromAlbumByName: " << errMes << std::endl;
        sqlite3_free(errMes);
    }

}

void DatabaseAccess::removeAllPictureFromAlbum(const std::string& albumName)
{
    char* errMes = nullptr;
    
    std::string sqlCmd = 
        "DELETE FROM PICTURES WHERE ALBUM_ID == (SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "')";

    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMes);
    if (res != SQLITE_OK) {
        std::cout << "SQL Error in removeAllPictureFromAlbum: " << errMes << std::endl;
        sqlite3_free(errMes);
    }
}



void DatabaseAccess::tagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
    char* errMes = nullptr;

    // because we not have the piecther id and just the the name of the alcume and the name we need the use slecet on the request it self to get the rigth picture id from the right album 
    std::string sqlCmd =
        "INSERT INTO TAGS (PICTURE_ID, USER_ID) SELECT ID, " + std::to_string(userId) +
        " FROM PICTURES WHERE NAME = '" + pictureName + "' AND ALBUM_ID = ("
        "SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "');";

    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMes);

    if (res != SQLITE_OK) { 
        std::cout << "SQL Error in tagUserInPicture: " << errMes << std::endl;
        sqlite3_free(errMes);
    }

}

void DatabaseAccess::untagUserInPicture(const std::string& albumName, const std::string& pictureName, int userId)
{
    char* errMes = nullptr;

    std::string sqlCmd =
        "DELETE FROM TAGS WHERE USER_ID = " + std::to_string(userId) +
        " AND PICTURE_ID = (SELECT ID FROM PICTURES WHERE NAME = '" + pictureName +
        "' AND ALBUM_ID = (SELECT ID FROM ALBUMS WHERE NAME = '" + albumName + "'));";

    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMes);

    if (res != SQLITE_OK) {
        std::cerr << "SQL Error in untagUserInPicture: " << errMes << std::endl;
        sqlite3_free(errMes);
    }
}


// User related
int DatabaseAccess::getLastUserId()
{
    int lastId = 0;
    sqlite3_stmt* stmt;

    std::string sql = "SELECT MAX(ID) FROM USERS;";

    if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            lastId = sqlite3_column_int(stmt, 0);
        }
    }
    sqlite3_finalize(stmt);
    return lastId;
}

void DatabaseAccess::printUsers()
{
    char* errMes = nullptr;
    sqlite3_stmt* stmt;
    std::string sqlCmd = "SELECT * FROM USERS;";
    int userI = 0;

    std::cout << "Users list:" << std::endl;
    std::cout << "-----------" << std::endl;
    if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int userId = sqlite3_column_int(stmt, 0);

            const unsigned char* userName = sqlite3_column_text(stmt, 1);

            User user(userId,std::string(reinterpret_cast<const char*>(userName)));

            std::cout << user << std::endl;
        }
    }
    sqlite3_finalize(stmt);
}

void DatabaseAccess::createUser(User& user)
{
    char* errMes = nullptr;

    std::string sqlCmd = "INSERT INTO USERS (NAME) VALUES ('" + user.getName() + "');";


    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMes);
    
    if (res != SQLITE_OK) {
        std::cout << "SQL Error in createUser: " << errMes << std::endl;
        sqlite3_free(errMes); 
    }
}

void DatabaseAccess::deleteUser(const User& user)
{
    unTagUserFromAllPicture(user);
    deleteAllbumsUser(user);


    char* errMes = nullptr;
    std::string sqlCmd = "DELETE FROM USERS WHERE ID = " + std::to_string(user.getId()) + ";";

    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMes);

    if (res != SQLITE_OK) {
        std::cout << "SQL Error in deleteUser: " << errMes << std::endl;
        sqlite3_free(errMes);
    }
}

void DatabaseAccess::deleteAllbumsUser(const User& user)
{
    //need to get the all albom users name to delete all the pictuer on them
    std::list<std::string> userAlbumsNames = getAlbumsNamesOfUser(user);

    for (const auto& albumName : userAlbumsNames) {
        removeAllPictureFromAlbum(albumName);
    }

    /// affter we raise all the piecter in album we can delete them from the db
    char* errMes = nullptr;
    std::string sqlCmd = "DELETE FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + ";";

    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMes);

    if (res != SQLITE_OK) {
        std::cerr << "SQL Error in deleteAllbumsUser: " << errMes << std::endl;
        sqlite3_free(errMes);
    }
}

void DatabaseAccess::unTagUserFromAllPicture(const User& user)
{

    char* errMes = nullptr;
    std::string sqlCmd = "DELETE FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + ";";

    int res = sqlite3_exec(_db, sqlCmd.c_str(), nullptr, nullptr, &errMes);

    if (res != SQLITE_OK) {
        std::cout << "SQL Error in deleteUser: " << errMes << std::endl;
        sqlite3_free(errMes);
    }
}

bool DatabaseAccess::doesUserExists(int userId)
{
    sqlite3_stmt* stmt;

    std::string sqlCmd = "SELECT NAME FROM USERS WHERE ID = " + std::to_string(userId) + ";";
    
    if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK)// if we success to make radey the commend we enter to loop to check each row if ther are a any row is say that we have user!
    {
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const unsigned char* name = sqlite3_column_text(stmt, 0);
            if (name)// we found a user whit this id!! so we reutrn true
            {
                sqlite3_finalize(stmt);
                return true;
            }
        }
    }
    sqlite3_finalize(stmt);
    return false;
}

User DatabaseAccess::getUser(int userId)
{
    sqlite3_stmt* stmt;


    string sqlcmd = "SELECT * FROM USERS WHERE ID = " + std::to_string(userId) + ";";
    
    if (sqlite3_prepare_v2(_db, sqlcmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            const char unsigned* userName = sqlite3_column_text(stmt, 1);


            User user(userId, std::string(reinterpret_cast<const char*>(userName)));
            sqlite3_finalize(stmt);
            return user;
        }
    }
    sqlite3_finalize(stmt);
    throw ItemNotFoundException("User", userId);
}

// User statistics
int DatabaseAccess::countAlbumsOwnedOfUser(const User& user)
{
    sqlite3_stmt* stmt;
    int count = 0;


    string sqlCmd = "SELECT COUNT(*) FROM ALBUMS WHERE USER_ID = " + std::to_string(user.getId()) + ";";

    if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return count;
}

int DatabaseAccess::countAlbumsTaggedOfUser(const User& user)
{
    sqlite3_stmt* stmt;
    int count = 0;
    

    string sqlCmd = "SELECT COUNT(DISTINCT ALBUM_ID) FROM PICTURES "
                     "JOIN TAGS ON PICTURES.ID = TAGS.PICTURE_ID "
                    "WHERE TAGS.USER_ID = " + std::to_string(user.getId()) + ";";


    if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return count;
}

int DatabaseAccess::countTagsOfUser(const User& user)
{
    sqlite3_stmt* stmt;
    int count = 0;


    std::string sqlCmd = "SELECT COUNT(*) FROM TAGS WHERE USER_ID = " + std::to_string(user.getId()) + ";";


    if (sqlite3_prepare_v2(_db, sqlCmd.c_str(), -1, &stmt, nullptr) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count = sqlite3_column_int(stmt, 0);
        }
    }

    sqlite3_finalize(stmt);
    return count;
}

float DatabaseAccess::averageTagsPerAlbumOfUser(const User& user)
{
    int totalTags = countTagsOfUser(user);
    int totalTaggedAlbums = countAlbumsTaggedOfUser(user);

    if (totalTaggedAlbums == 0) {
        return 0;
    }
    return static_cast<float>(totalTags) / totalTaggedAlbums;
}

// Queries
User DatabaseAccess::getTopTaggedUser() 
{
    sqlite3_stmt* stmt;
    User topUser(-1, "");

    std::string sql = "SELECT USERS.ID, USERS.NAME FROM USERS " // get the  name and the id form the the user but whit order whit the id in tags to get the most teged users and i limet the output to one users beacuse we need only one...
        "JOIN TAGS ON USERS.ID = TAGS.USER_ID "
        "GROUP BY TAGS.USER_ID "
        "ORDER BY COUNT(TAGS.USER_ID) DESC LIMIT 1;";

    if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {// becuase we have a 1 row of user we dont need to user loop
            int id = sqlite3_column_int(stmt, 0);
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            topUser = User(id, name);
        }
    }
    sqlite3_finalize(stmt);
    return topUser;
}

Picture DatabaseAccess::getTopTaggedPicture()
{
    sqlite3_stmt* stmt;
    Picture topPic(0,"","","");

    std::string sql = "SELECT PICTURES.ID, PICTURES.NAME, PICTURES.PATH, PICTURES.CREATION_DATE "
        "FROM PICTURES JOIN TAGS ON PICTURES.ID = TAGS.PICTURE_ID "
        "GROUP BY PICTURES.ID "
        "ORDER BY COUNT(TAGS.PICTURE_ID) DESC LIMIT 1;";

    if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            const char* date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            topPic = Picture(id, name, path, date);
        }
    }
    sqlite3_finalize(stmt);
    return topPic;
}

std::list<Picture> DatabaseAccess::getTaggedPicturesOfUser(const User& user)
{
    std::list<Picture> pictures;
    sqlite3_stmt* stmt;

    std::string sql = "SELECT PICTURES.ID, PICTURES.NAME, PICTURES.PATH, PICTURES.CREATION_DATE "
        "FROM PICTURES JOIN TAGS ON PICTURES.ID = TAGS.PICTURE_ID "
        "WHERE TAGS.USER_ID = " + std::to_string(user.getId()) + ";";

    if (sqlite3_prepare_v2(_db, sql.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int id = sqlite3_column_int(stmt, 0);
            const char* name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            const char* path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            const char* date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));

            pictures.push_back(Picture(id, name, path, date));
        }
    }
    sqlite3_finalize(stmt);
    return pictures;
}
