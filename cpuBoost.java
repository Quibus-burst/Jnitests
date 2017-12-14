#include <cpuBoost.h>
public class cpuBoost {
	public native long[] calculate(long[] acc,long[] id,int[] scoopnr);
	static {
		System.loadLibrary("cpuBoost");
	}
}