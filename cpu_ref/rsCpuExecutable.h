/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_RENDERSCRIPT_EXECUTABLE_H
#define ANDROID_RENDERSCRIPT_EXECUTABLE_H

#include <stdlib.h>

#include "rsCpuScript.h"

namespace android {
namespace renderscript {

class Context;

class SharedLibraryUtils {
public:
#ifndef RS_COMPATIBILITY_LIB
    static bool createSharedLibrary(const char* cacheDir, const char* resName);
#endif

    // Load the shared library referred to by cacheDir and resName. If we have
    // already loaded this library, we instead create a new copy (in the
    // cache dir) and then load that. We then immediately destroy the copy.
    // This is required behavior to implement script instancing for the support
    // library, since shared objects are loaded and de-duped by name only.

    // For 64bit RS Support Lib, the shared lib path cannot be constructed from
    // cacheDir, so nativeLibDir is needed to load shared libs.
    static void* loadSharedLibrary(const char *cacheDir, const char *resName,
                                   const char *nativeLibDir = nullptr);

private:
    // Attempt to load the shared library from origName, but then fall back to
    // creating a copy of the shared library if necessary (to ensure instancing).
    // This function returns the dlopen()-ed handle if successful.
    static void *loadSOHelper(const char *origName, const char *cacheDir,
                              const char *resName);

    static const char* LD_EXE_PATH;
    static const char* RS_CACHE_DIR;
};

class ScriptExecutable {
public:
    ScriptExecutable(Context* RSContext,
                     void** fieldAddress, bool* fieldIsObject, size_t varCount,
                     InvokeFunc_t* invokeFunctions, size_t funcCount,
                     ForEachFunc_t* forEachFunctions, uint32_t* forEachSignatures,
                     size_t forEachCount,
                     const char ** pragmaKeys, const char ** pragmaValues,
                     size_t pragmaCount,
                     bool isThreadable) :
        mFieldAddress(fieldAddress), mFieldIsObject(fieldIsObject),
            mExportedVarCount(varCount),
            mInvokeFunctions(invokeFunctions), mFuncCount(funcCount),
            mForEachFunctions(forEachFunctions), mForEachSignatures(forEachSignatures),
            mForEachCount(forEachCount),
            mPragmaKeys(pragmaKeys), mPragmaValues(pragmaValues),
            mPragmaCount(pragmaCount),
            mIsThreadable(isThreadable), mRS(RSContext) {
    }

    ~ScriptExecutable() {
        for (size_t i = 0; i < mExportedVarCount; ++i) {
            if (mFieldIsObject[i]) {
                if (mFieldAddress[i] != nullptr) {
                    rs_object_base *obj_addr =
                            reinterpret_cast<rs_object_base *>(mFieldAddress[i]);
                    rsrClearObject(mRS, obj_addr);
                }
            }
        }

        for (size_t i = 0; i < mPragmaCount; ++i) {
            delete [] mPragmaKeys[i];
            delete [] mPragmaValues[i];
        }

        delete[] mPragmaValues;
        delete[] mPragmaKeys;
        delete[] mForEachSignatures;
        delete[] mForEachFunctions;
        delete[] mInvokeFunctions;
        delete[] mFieldIsObject;
        delete[] mFieldAddress;
    }

    static ScriptExecutable*
            createFromSharedObject(Context* RSContext, void* sharedObj);

    size_t getExportedVariableCount() const { return mExportedVarCount; }
    size_t getExportedFunctionCount() const { return mFuncCount; }
    size_t getExportedForEachCount() const { return mForEachCount; }
    size_t getPragmaCount() const { return mPragmaCount; }

    void* getFieldAddress(int slot) const { return mFieldAddress[slot]; }
    bool getFieldIsObject(int slot) const { return mFieldIsObject[slot]; }
    InvokeFunc_t getInvokeFunction(int slot) const { return mInvokeFunctions[slot]; }
    ForEachFunc_t getForEachFunction(int slot) const { return mForEachFunctions[slot]; }
    uint32_t getForEachSignature(int slot) const { return mForEachSignatures[slot]; }

    const char ** getPragmaKeys() const { return mPragmaKeys; }
    const char ** getPragmaValues() const { return mPragmaValues; }

    bool getThreadable() const { return mIsThreadable; }

private:
    void** mFieldAddress;
    bool* mFieldIsObject;
    size_t mExportedVarCount;

    InvokeFunc_t* mInvokeFunctions;
    size_t mFuncCount;

    ForEachFunc_t* mForEachFunctions;
    uint32_t* mForEachSignatures;
    size_t mForEachCount;

    const char ** mPragmaKeys;
    const char ** mPragmaValues;
    size_t mPragmaCount;

    bool mIsThreadable;

    Context* mRS;
};

}  // namespace renderscript
}  // namespace android

#endif  // ANDROID_RENDERSCRIPT_EXECUTABLE_H