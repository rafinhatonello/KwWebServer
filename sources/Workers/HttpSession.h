//This cla sses uses the library JsonMaker: https://github.com/SerraFullStack/libs.json_maker
#ifndef HTTPSESSION_H
#define HTTPSESSION_H

#include "../../../libs.json_maker/cpp/JSON.h"
#include "../KWTinyWebServer.h"
#include "../IWorker.h"


namespace KWShared{
    using namespace JsonMaker;
    class HttpSession: public IWorker
    {
        private:
            unsigned int uniqueCount  = 0;
            StringUtils strUtils;
            string getFileNameFromSsid(string ssid)
            {
                string nId = "";
                string validChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_";
                for (auto &c: ssid)
                {
                    if (validChars.find(c) != string::npos)
                        nId += c;
                    else
                        nId += "_";
                }
                return this->dataFolder + nId +".json";
            }

            string getUniqueId(){
                //prepare a unique data
                string hasgUniqueData = "";
                hasgUniqueData += uniqueCount;
                hasgUniqueData += strUtils.formatDate(time(NULL));

                //calculates the SHA1 hash of the prepared data
                unsigned char sha1result[SHA_DIGEST_LENGTH];
                SHA1((unsigned char*)hasgUniqueData.c_str(), hasgUniqueData.size(), sha1result);
                string sha1Result = this->strUtils.base64_encode(sha1result, SHA_DIGEST_LENGTH);

                sha1Result+= uniqueCount++;
                sha1Result+="\0";


                //return the SHA1 as result
                return sha1Result;
            }

            SysLink sysLink;
            string dataFolder;

        public:
            void start(void* webserver){
                SysLink sysLink;

                this->dataFolder = ((KWTinyWebServer*)webserver)->__dataFolder + "/sessions/";
                if (!sysLink.directoryExists(this->dataFolder))
                    sysLink.createDirectory(this->dataFolder);

            };

            void load(HttpData* httpData){ }

            void unload(HttpData* httpData){ }

            void* setSessionData(HttpData* httpData, JSON* session){
                //find session object
                string ssid = "";
                if (httpData->cookies.find("ssid") == httpData->cookies.end() || httpData->cookies["ssid"] == NULL)
                {
                    //cout << "Created a new object for session data" <<  endl;
                    //create a cookie with a new ssid
                    ssid = this->getUniqueId();

                    httpData->cookies["ssid"] = new HttpCookie("ssid", ssid);

                }

                //get the ssid
                ssid = httpData->cookies["ssid"]->value;

                string sessionFileName = getFileNameFromSsid(ssid);

                //write data to file

                sysLink.waitAndLockFile(sessionFileName, 5000);
                    //load existing data
                    string existingData = sysLink.readFile(sessionFileName);
                    JSON* tmp = new JSON(existingData);
                    //apenddata
                    tmp->parseJson(session->ToJson());
                    string data = tmp->ToJson();
                    tmp->clear();

                    this->sysLink.writeFile(sessionFileName, data);
                sysLink.unlockFile(sessionFileName);
            }

            JSON* getSessionData(HttpData* httpData)
            {
                /*for (auto &c :httpData->cookies)
                {
                    cout << "get session cookie: " << c.first << "="<< c.second->value << endl;
                }*/
                string ssid = "";

                if (httpData->cookies.find("ssid") == httpData->cookies.end() || httpData->cookies["ssid"] == NULL)
                {
                    //cout << "Created a new object for session data" <<  endl;
                    //create a cookie with a new ssid
                    ssid = this->getUniqueId();

                    httpData->cookies["ssid"] = new HttpCookie("ssid", ssid);

                }

                ssid = httpData->cookies["ssid"]->value;

                //ensures 'ssid' cookie security
                httpData->cookies["ssid"]->secure = false;



                //determine the fiename of file with session data
                string sessionFileName = getFileNameFromSsid(ssid);

                //create a new json to receive the data of the current session
                JSON* tmp = new JSON();
                if (this->sysLink.fileExists(sessionFileName))
                {
                    sysLink.waitAndLockFile(sessionFileName, 5000);
                    string dt = this->sysLink.readFile(sessionFileName);
                    sysLink.unlockFile(sessionFileName);
                    tmp->parseJson(dt);
                    dt.clear();
                }

                return tmp;

            }
    };
}

#endif // HTTPSESSION_H
