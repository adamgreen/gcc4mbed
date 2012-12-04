/* mbed Microcontroller Library
 * Copyright (c) 2006-2012 ARM Limited
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef MBED_FUNCTIONPOINTER_H
#define MBED_FUNCTIONPOINTER_H

#include <string.h>

namespace mbed { 

/** A class for storing and calling a pointer to a static or member void function
 */
class FunctionPointer {
public:

    /** Create a FunctionPointer, attaching a static function
     * 
     *  @param function The void static function to attach (default is none)
     */
    FunctionPointer(void (*function)(void) = 0);

    /** Create a FunctionPointer, attaching a member function
     * 
     *  @param object The object pointer to invoke the member function on (i.e. the this pointer)
     *  @param function The address of the void member function to attach 
     */
    template<typename T>
    FunctionPointer(T *object, void (T::*member)(void)) {
        attach(object, member);
    }
    
    /** Attach a static function
     * 
     *  @param function The void static function to attach (default is none)
     */
    void attach(void (*function)(void) = 0);
    
    /** Attach a member function
     * 
     *  @param object The object pointer to invoke the member function on (i.e. the this pointer)
     *  @param function The address of the void member function to attach 
     */
    template<typename T>
    void attach(T *object, void (T::*member)(void)) {
        _object = static_cast<void*>(object);
        memcpy(_member, (char*)&member, sizeof(member));
        _membercaller = &FunctionPointer::membercaller<T>;
        _function = 0;
    }
    
    /** Call the attached static or member function
     */
    void call();

private:
    template<typename T>
    static void membercaller(void *object, char *member) {
        T* o = static_cast<T*>(object);
        void (T::*m)(void);
        memcpy((char*)&m, member, sizeof(m));
        (o->*m)();
    }
    
    void (*_function)(void);                // static function pointer - 0 if none attached
    void *_object;                            // object this pointer - 0 if none attached
    char _member[16];                        // raw member function pointer storage - converted back by registered _membercaller
    void (*_membercaller)(void*, char*);    // registered membercaller function to convert back and call _member on _object
};

} // namespace mbed

#endif
