#include <tchar.h>
#include <windows.h>
#include <memory>
#include <iostream>
#include "jni.h"

#define __cpp_defer_stringify_(a,b,c,d)		a##b##c##d
#define __cpp_defer_stringify(a,b,c,d)		__cpp_defer_stringify_(a,b,c,d)

#define defer(type, ptr, func)										\
std::shared_ptr<type> __cpp_defer_stringify(__defer,type,ptr,__LINE__)(ptr, [](type *p) {		\
    if (p) {func(p);}    });

#define defer_no_check(type, ptr, func)									\
std::shared_ptr<type> __cpp_defer_stringify(__defer,type,ptr,__LINE__)(ptr, [](type *p) {		\
    func(p);    });

#define defer_raw(type, ptr,block)									\
std::shared_ptr<type> __cpp_defer_stringify(__defer,type,ptr,__LINE__)(ptr, [&](type *p) {		\
    block;    });

using namespace std;


void doTestCPP1(JNIEnv* e, jobject o) {
	std::cout << "C++callback activated1" << std::endl;
	jfieldID f_uid = e->GetFieldID(e->GetObjectClass(o), "uid", "I");
	if (f_uid)
		std::cout << "UID data member: " << e->GetIntField(o, f_uid) << std::endl;
	else std::cout << "UID not found" << std::endl;
}

int doTestCPP2(JNIEnv* e, jobject o, int param1) {
	std::cout << "C++callback activated2, param: " << param1 << std::endl;
	jfieldID f_uid = e->GetFieldID(e->GetObjectClass(o), "uid", "I");
	if (f_uid)
		std::cout << "UID data member: " << e->GetIntField(o, f_uid) << std::endl;
	else std::cout << "UID not found" << std::endl;
	return 456;
}


int main()
{
	TCHAR jvm_path[MAX_PATH] = L"C:\\Program Files (x86)\\Java\\jdk1.8.0_251\\jre\\bin\\client\\jvm.dll";
	HMODULE handle = LoadLibrary(jvm_path);
	if (!handle)
		return -1;
	defer_raw(HMODULE, NULL, FreeLibrary(handle));

	typedef jint (JNICALL * JNI_CreateJavaVMT)(JavaVM * *pvm, void** penv, void* args);
	JNI_CreateJavaVMT pJNI_CreateJavaVM = (JNI_CreateJavaVMT)GetProcAddress(handle, "JNI_CreateJavaVM");
	if (!pJNI_CreateJavaVM)
		return -2;

	JavaVM	*jvm;				// 代码：JVM 指针(Java Virtual Machine)
	JNIEnv	*env;				// 数据：jar等程序环境

	JavaVMInitArgs vm_args;
	JavaVMOption options[2];
	options[0].optionString = (char *)"-Djava.class.path=.;D:/MyTest.jar";	//这里指定了要使用的第三方Jar包  
	options[1].optionString = (char*)"-verbose:NONE";			//用于跟踪运行时的信息  
	vm_args.version = JNI_VERSION_1_6;	// 最低jvm版本
	vm_args.options = options;
	vm_args.nOptions = sizeof(options) / sizeof(JavaVMOption);
	vm_args.ignoreUnrecognized = JNI_FALSE;	// 如果optionString错误，jvm虚拟机会创建失败，建议false
	jint res = pJNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);
	if (res != JNI_OK) {
		printf("create jvm failed, please check vm_args.\n");
		return -3;
	}
	printf("create jvm success.\n");
	jint ver = env->GetVersion();
	printf("jvm version: %d.%d\n", ((ver >> 16) & 0x0f), (ver & 0x0f));
	jclass myclass = env->FindClass("MyTest");  // try to find the class 
	if (myclass != nullptr) {
		// 测试1：静态函数无参数
		jmethodID mid1 = env->GetStaticMethodID(myclass, "mymain1", "()V");
		if (mid1 != nullptr)
			env->CallStaticVoidMethod(myclass, mid1);
		// 测试2：静态函数有一个int参数
		jmethodID mid2 = env->GetStaticMethodID(myclass, "mymain2", "(I)I");
		if (mid2 != nullptr)
			env->CallStaticVoidMethod(myclass, mid2, (jint)5);
		// 测试3：静态函数有一个string数组参数
		jmethodID mid3 = env->GetStaticMethodID(myclass, "main", "([Ljava/lang/String;)V");
		if (mid2 != nullptr) {
			jobjectArray arr = env->NewObjectArray(5	// 数组大小是5
				, env->FindClass("java/lang/String")	// 数组类型是String
				, env->NewStringUTF("my_str"));		// 每个数组项的初始值是"my_str"
			env->SetObjectArrayElement(arr, 1, env->NewStringUTF("MYOWNSTRING"));	// 改变数组成员1
			env->CallStaticVoidMethod(myclass, mid3, arr);	// 调用java接口
			env->DeleteLocalRef(arr);			// 释放资源
		}
	}

	// 自定义方法注入虚拟机的类中
	JNINativeMethod methods[]{
		{   (char *)"doTest1", (char*)"()V",  &doTestCPP1 }
		, { (char *)"doTest2", (char*)"(I)I", &doTestCPP2 }
	};
	int func_count = sizeof(methods) / sizeof(JNINativeMethod);
	res = env->RegisterNatives(myclass, methods, func_count);
	if (JNI_OK == res) {
		// 测试4：非静态函数（c中新建java对象，然后调用），以及在这个函数里面回调c代码
		// 回调（1）不带参数（2）带参数
		// c中的回调建议，立即返回，如果耗时则考虑用线程来封装

		// (1)查找构造函数：无参数
		jmethodID ctor = env->GetMethodID(myclass, "<init>", "()V");
		if (ctor != nullptr) {
			jobject myobject = env->NewObject(myclass, ctor);
			if (myobject) {
				jmethodID show = env->GetMethodID(myclass, "showId", "()V");
				if (show != nullptr) {
					env->CallVoidMethod(myobject, show);
				}
			}
		}
	} else {
		printf("fetal error: register function to jvm failed.\n");
	}
	jvm->DestroyJavaVM();
}

