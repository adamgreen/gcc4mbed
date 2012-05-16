/* Copyright 2011 Adam Green (http://mbed.org/users/AdamGreen/)

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
/* Header file for debug related functionality. */
#ifndef DEBUG_H_
#define DEBUG_H_


#include <mbed.h>


// Assert related macroes and functions.
#ifndef NDEBUG
    static __inline int __AssertLog(const char* pAssert, const char* pFilename, unsigned int LineNumber)
    {
        static const char  UsersDir[] = "/users/";
        const char*        pUsers;
        
        // Skip long workspace source filename prefix.
        pUsers = strstr(pFilename, UsersDir);
        if (pUsers)
        {
            pUsers = strchr(pUsers + sizeof(UsersDir) - 1, '/');
            if (pUsers)
            {
                pFilename = pUsers + 1;
            }
        }
   
        error("%s:%u Assertion failure: %s\r\n", pFilename, LineNumber, pAssert);
        return 0;
    }
    #define assert(X) ((X) || __AssertLog(#X, __FILE__, __LINE__))
#else
    static __inline void __AssertLog()
    {
    }
    #define assert(X) __AssertLog()
#endif // NDEBUG



#endif /* DEBUG_H_ */
