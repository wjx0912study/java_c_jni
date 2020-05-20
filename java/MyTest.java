public class MyTest
{
    private static int magic_counter = 777;

	// c调用java：静态函数无参数
    public static void mymain1()
    {
        System.out.println("Hello, World in java from mymain");
        System.out.println(magic_counter);
    }

    // c调用java：静态函数带参数，有返回值
    public static int mymain2(int n)
    {
        for (int i = 0; i < n; i++)
        {
            System.out.print (i);
            System.out.println("Hello, World !");
        }
        return n * 2;
    }

    public static void main (String[] args)
    {
        System.out.println("Hello, World in java");
        int i;
        for (i = 0; i < args.length; i++)
            System.out.println(args[i]);
    }

    //<=========================== 
	// 构造函数
    private int uid = 123;					// 私有成员
    public MyTest()
    {
        uid = magic_counter++ * 2;
    }

	// java回调c：有参数，无参数，暂不考虑返回值
    public void showId()
    {
        System.out.println(uid);
        doTest1();
		uid++;
        int result = doTest2(123);
		System.out.println("result from c language: " + result);
    }

    public native void doTest1();			// c语言JNI实现
    public native int doTest2(int n);		// c语言JNI实现
}

