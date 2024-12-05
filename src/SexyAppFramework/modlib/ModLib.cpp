#define XMD_H

#include "../Common.h"
#include "ModLib.h"

#include <math.h>
#include "SexyAppFramework/paklib/PakInterface.h"


using namespace ModLib;

Mod::Mod()
{
	mName = nullptr;
	mVersion = 0;
	mBits = NULL;
}

Mod::~Mod()
{
	delete mBits;
}

std::string Mod::GetName()
{
	return mName;
}

int	Mod::GetVersion()
{
	return mVersion;
}

uint32_t* Mod::GetBits()
{
	return mBits;
}

//////////////////////////////////////////////////////////////////////////
// PNG Pak Support
/*
static void mod_pak_read_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
	png_size_t check;


	check = (png_size_t)p_fread(data, (png_size_t)1, length,
		(PFILE*)png_get_io_ptr(png_ptr));

	if (check != length)
	{
		png_error(png_ptr, "Read Error");
	}
}
*/
#include <windows.h>

Mod* GetDLLMod(const std::string& theFileName) {
    // 加载 DLL 文件
    HMODULE hModule = LoadLibrary(theFileName.c_str());
    if (!hModule) {
        return nullptr; // 加载失败
    }

    // 创建并返回图像对象 Mod
    Mod* anMod = new Mod();
    anMod->mName = nullptr; // 根据您的需求设置宽度
    anMod->mVersion = 0; // 根据您的需求设置高度
    anMod->mBits = nullptr; // 根据您的需求初始化 mBits

    // 这里可以根据 DLL 的内容进行更多的处理，比如获取函数指针或调用特定的函数
    // 例如假设 DLL 中有一函数叫 "InitializeMod"，可以如下调用：
    // typedef void (*InitializeModFunc)(Mod* mod);
    // InitializeModFunc initFunc = (InitializeModFunc)GetProcAddress(hModule, "InitializeMod");
    // if (initFunc) {
    //     initFunc(anMod); // 调用 DLL 中的函数
    // }

    // 以及其他与 DLL 相关的逻辑

    // 返回 Mod 对象
    return anMod;
}



Mod* GetSOMod(const std::string& theFileName) {
    PFILE* aTGAFile = p_fopen(theFileName.c_str(), "rb");
    if (!aTGAFile) return nullptr;

    // 读取TGA头信息
    BYTE aHeader[18]; // TGA头部固定长度为18字节
    p_fread(aHeader, sizeof(BYTE), 18, aTGAFile);

    // 提取重要信息
    WORD anModWidth = *((WORD*)&aHeader[12]);
    WORD anModHeight = *((WORD*)&aHeader[14]);
    BYTE aBitCount = aHeader[16];
    BYTE anModDescriptor = aHeader[17];

    // 检查图像格式
    if (aBitCount != 32 || (anModDescriptor != (8 | (1 << 5)))) {
        p_fclose(aTGAFile);
        return nullptr;
    }

    // 创建图像对象并读取像素数据
    Mod* anMod = new Mod();
    anMod->mName = anModWidth;
    anMod->mVersion = anModHeight;
    anMod->mBits = new uint32_t[anModWidth * anModHeight];

    p_fread(anMod->mBits, sizeof(uint32_t), 1, aTGAFile);
    p_fclose(aTGAFile);

    return anMod;
}

int ReadModBlobBlock(PFILE* fp, char* data)
{
	unsigned char aCount = 0;
	p_fread(&aCount, sizeof(char), 1, fp);
	p_fread(data, sizeof(char), aCount, fp);
	return aCount;
}

Mod* GetGIFMod(const std::string& theFileName)
{
	#define BitSet(byte,bit)  (((byte) & (bit)) == (bit))
	#define LSBFirstOrder(x,y)  (((y) << 8) | (x))

	int
		opacity,
		status;

	int i;

	unsigned char *p;

	unsigned char
		background,			// 背景色在全局颜色列表中的索引（背景色：图像中没有被指定颜色的像素会被背景色填充）
		c,
		flag,				// 图像标志的压缩字节
		*global_colormap,	// 全局颜色列表
		header[1664],
		magick[12];

	unsigned int
		delay,
		dispose,
		global_colors,		// 全局颜色列表大小
		image_count,
		iterations;

	/*
	Open image file.
	*/

	PFILE *fp;

	if ((fp = p_fopen(theFileName.c_str(), "rb")) == NULL)
		return NULL;
	/*
	Determine if this is a GIF file.
	*/
	status = p_fread(magick, sizeof(char), 6, fp);  // 读取文件头（包含文件签名与版本号，共 6 字节）
	(void)status; // unused

	// 文件头的 ASCII 值为“GIF87a”或”GIF89a”，其中前三位为 GIF 签名，后三位为不同年份的版本号
	if (((strncmp((char*)magick, "GIF87", 5) != 0) && (strncmp((char*)magick, "GIF89", 5) != 0)))
		return NULL;

	global_colors = 0;
	global_colormap = (unsigned char*)NULL;

	short pw;  // 图像宽度
	short ph;  // 图像高度

	// 读取逻辑屏幕描述符，共 7 字节
	p_fread(&pw, sizeof(short), 1, fp);  // 读取图像渲染区域的宽度
	p_fread(&ph, sizeof(short), 1, fp);  // 读取图像渲染区域的高度
	p_fread(&flag, sizeof(char), 1, fp);  // 读取图像标志
	p_fread(&background, sizeof(char), 1, fp);  // 读取背景色在全局颜色列表中的索引，若无全局颜色列表则此字节无效
	p_fread(&c, sizeof(char), 1, fp);  // 读取像素宽高比

	if (BitSet(flag, 0x80))  // 如果存在全局颜色列表
	{
		/*
		opacity global colormap.
		*/
		global_colors = 1 << ((flag & 0x07) + 1);  // 压缩字节的最低 3 位表示全局颜色列表的大小，设其二进制数值为 N，则列表大小 = 2 ^ (N + 1)
		global_colormap = new unsigned char[3 * global_colors];  // 每个颜色占 3 个字节，按 RGB 排列
		if (global_colormap == (unsigned char*)NULL)
			return NULL;

		p_fread(global_colormap, sizeof(char), 3 * global_colors, fp);  // 读取全局颜色列表
	}

	delay = 0;
	dispose = 0;
	iterations = 1;
	opacity = (-1);
	image_count = 0;

	for (; ; )
	{
		if (p_fread(&c, sizeof(char), 1, fp) == 0)
			break;  // 如果读取错误或读取到文件尾则退出，返回空指针

		if (c == ';')  // 当读取到 gif 结束块标记符（End Of File）
			break;  /* terminator */
		if (c == '!')  // 当读取到 gif 拓展块标记符
		{
			/*
			GIF Extension block.
			*/
			p_fread(&c, sizeof(char), 1, fp);  // 读取拓展块的功能编码号

			switch (c)
			{
			case 0xf9:
			{
				/*
				Read Graphics Control extension.
				*/
				while (ReadModBlobBlock(fp, (char*)header) > 0);

				dispose = header[0] >> 2;
				delay = (header[2] << 8) | header[1];
				(void)delay; // Unused
				if ((header[0] & 0x01) == 1)
					opacity = header[3];
				break;
			}
			case 0xfe:
			{
				char* comments;
				int length;

				/*
				Read Comment extension.
				*/
				comments = (char*)NULL;
				for (; ; )
				{
					length = ReadModBlobBlock(fp, (char*)header);
					if (length <= 0)
						break;
					if (comments == NULL)
					{
						comments = new char[length + 1];
						if (comments != (char*)NULL)
							*comments = '\0';
					}

					header[length] = '\0';
					strcat(comments, (char*)header);
				}
				if (comments == (char*)NULL)
					break;

				delete comments;
				break;
			}
			case 0xff:
			{
				int
					loop;

				/*
				Read Netscape Loop extension.
				*/
				loop = false;
				if (ReadModBlobBlock(fp, (char*)header) > 0)
					loop = !strncmp((char*)header, "NETSCAPE2.0", 11);
				while (ReadModBlobBlock(fp, (char*)header) > 0)
					if (loop)
						iterations = (header[2] << 8) | header[1];
				break;
			}
			default:
			{
				while (ReadModBlobBlock(fp, (char*)header) > 0);
				break;
			}
			}
		}

		if (c != ',')  // 如果读取的不为图像描述符
			continue;

		if (image_count != 0)
		{
			/*
			Allocate next image structure.
			*/

			/*AllocateNextMod(image_info,image);
			if (image->next == (Mod *) NULL)
			{
			DestroyMods(image);
			return((Mod *) NULL);
			}
			image=image->next;
			MagickMonitor(LoadModsText,TellBlob(image),image->filesize);*/
		}
		image_count++;

		short pagex;
		short pagey;
		short width;
		short height;
		int colors;
		bool interlaced;

		p_fread(&pagex, sizeof(short), 1, fp);  // 读取帧的横坐标（Left）
		p_fread(&pagey, sizeof(short), 1, fp);  // 读取帧的纵坐标（Top）
		p_fread(&width, sizeof(short), 1, fp);  // 读取帧的横向宽度（Width）
		p_fread(&height, sizeof(short), 1, fp);  // 取得帧的纵向高度（Height）
		p_fread(&flag, sizeof(char), 1, fp);  // 读取帧标志的压缩字节

		colors = !BitSet(flag, 0x80) ? global_colors : 1 << ((flag & 0x07) + 1);  // 判断使用全局颜色列表或使用局部颜色列表，并取得列表大小
		uint32_t* colortable = new uint32_t[colors];  // 申请颜色列表

		interlaced = BitSet(flag, 0x40);  // 当前帧图像数据存储方式，为 1 表示交织顺序存储，0 表示顺序存储

		delay = 0;
		dispose = 0;
		(void)dispose; // unused
		iterations = 1;
		(void)iterations; //unused
		/*if (image_info->ping)
		{
		f (opacity >= 0)
		/image->matte=true;

		CloseBlob(image);
		return(image);
		}*/
		if ((width == 0) || (height == 0))
			return NULL;
		/*
		Inititialize colormap.
		*/
		/*if (!AllocateModColormap(image,image->colors))
		ThrowReaderException(ResourceLimitWarning,"Memory allocation failed",
		image);*/
		if (!BitSet(flag, 0x80))  // 如果使用全局颜色列表
		{
			/*
			Use global colormap.
			*/
			p = global_colormap;
			for (i = 0; i < (int)colors; i++)
			{
				int r = *p++;
				int g = *p++;
				int b = *p++;

				colortable[i] = 0xFF000000 | (r << 16) | (g << 8) | (b);
			}

			//image->background_color=
			//image->colormap[Min(background,image->colors-1)];
		}
		else
		{
			unsigned char
				* colormap;

			/*
			Read local colormap.
			*/
			colormap = new unsigned char[3 * colors];

			int pos = p_ftell(fp);
			(void)pos; // unused

			p_fread(colormap, sizeof(char), 3 * colors, fp);

			p = colormap;
			for (i = 0; i < (int)colors; i++)
			{
				int r = *p++;
				int g = *p++;
				int b = *p++;

				colortable[i] = 0xFF000000 | (r << 16) | (g << 8) | (b);
			}
			delete colormap;
		}

		/*if (opacity >= (int) colors)
		{
		for (i=colors; i < (opacity+1); i++)
		{
		image->colormap[i].red=0;
		image->colormap[i].green=0;
		image->colormap[i].blue=0;
		}
		image->colors=opacity+1;
		}*/
		/*
		Decode image.
		*/
		//status=DecodeMod(image,opacity,exception);

		//if (global_colormap != (unsigned char *) NULL)
		// LiberateMemory((void **) &global_colormap);
		if (global_colormap != NULL)
		{
			delete[] global_colormap;
			global_colormap = NULL;
		}

		//while (image->previous != (Mod *) NULL)
		//    image=image->previous;

#define MaxStackSize  4096
#define NullCode  (-1)

		int
			available,
			bits,
			code,
			clear,
			code_mask,
			code_size,
			count,
			end_of_information,
			in_code,
			offset,
			old_code,
			pass,
			y;

		int
			x;

		unsigned int
			datum;

		short
			* prefix;

		unsigned char
			data_size,
			first,
			* packet,
			* pixel_stack,
			* suffix,
			* top_stack;

		/*
		Allocate decoder tables.
		*/

		packet = new unsigned char[256];
		prefix = new short[MaxStackSize];
		suffix = new unsigned char[MaxStackSize];
		pixel_stack = new unsigned char[MaxStackSize + 1];

		/*
		Initialize GIF data stream decoder.
		*/
		p_fread(&data_size, sizeof(char), 1, fp);
		clear = 1 << data_size;
		end_of_information = clear + 1;
		available = clear + 2;
		old_code = NullCode;
		code_size = data_size + 1;
		code_mask = (1 << code_size) - 1;
		for (code = 0; code < clear; code++)
		{
			prefix[code] = 0;
			suffix[code] = (unsigned char)code;
		}
		/*
		Decode GIF pixel stream.
		*/
		datum = 0;
		bits = 0;
		c = 0;
		count = 0;
		first = 0;
		offset = 0;
		pass = 0;
		top_stack = pixel_stack;

		uint32_t* aBits = new uint32_t[width * height];

		unsigned char* c = NULL;

		for (y = 0; y < (int)height; y++)
		{
			//q=SetModPixels(image,0,offset,width,1);
			//if (q == (PixelPacket *) NULL)
			//break;
			//indexes=GetIndexes(image);

			uint32_t* q = aBits + offset * width;



			for (x = 0; x < (int)width; )
			{
				if (top_stack == pixel_stack)
				{
					if (bits < code_size)
					{
						/*
						Load bytes until there is enough bits for a code.
						*/
						if (count == 0)
						{
							/*
							Read a new data block.
							*/
							int pos = p_ftell(fp);
							(void)pos; // unused

							count = ReadModBlobBlock(fp, (char*)packet);
							if (count <= 0)
								break;
							c = packet;
						}
						datum += (*c) << bits;
						bits += 8;
						c++;
						count--;
						continue;
					}
					/*
					Get the next code.
					*/
					code = datum & code_mask;
					datum >>= code_size;
					bits -= code_size;
					/*
					Interpret the code
					*/
					if ((code > available) || (code == end_of_information))
						break;
					if (code == clear)
					{
						/*
						Reset decoder.
						*/
						code_size = data_size + 1;
						code_mask = (1 << code_size) - 1;
						available = clear + 2;
						old_code = NullCode;
						continue;
					}
					if (old_code == NullCode)
					{
						*top_stack++ = suffix[code];
						old_code = code;
						first = (unsigned char)code;
						continue;
					}
					in_code = code;
					if (code >= available)
					{
						*top_stack++ = first;
						code = old_code;
					}
					while (code >= clear)
					{
						*top_stack++ = suffix[code];
						code = prefix[code];
					}
					first = suffix[code];
					/*
					Add a new string to the string table,
					*/
					if (available >= MaxStackSize)
						break;
					*top_stack++ = first;
					prefix[available] = old_code;
					suffix[available] = first;
					available++;
					if (((available & code_mask) == 0) && (available < MaxStackSize))
					{
						code_size++;
						code_mask += available;
					}
					old_code = in_code;
				}
				/*
				Pop a pixel off the pixel stack.
				*/
				top_stack--;

				int index = (*top_stack);

				*q = colortable[index];

				if (index == opacity)
					*q = 0;

				x++;
				q++;
			}

			if (!interlaced)
				offset++;
			else
			{
				switch (pass)
				{
				case 0:
				default:
				{
					offset += 8;
					if (offset >= height)
					{
						pass++;
						offset = 4;
					}
					break;
				}
				case 1:
				{
					offset += 8;
					if (offset >= height)
					{
						pass++;
						offset = 2;
					}
					break;
				}
				case 2:
				{
					offset += 4;
					if (offset >= height)
					{
						pass++;
						offset = 1;
					}
					break;
				}
				case 3:
				{
					offset += 2;
					break;
				}
				}
			}

			if (x < width)
				break;

			/*if (image->previous == (Mod *) NULL)
			if (QuantumTick(y,image->rows))
			MagickMonitor(LoadModText,y,image->rows);*/
		}
		delete pixel_stack;
		delete suffix;
		delete prefix;
		delete packet;

		delete[] colortable;

		//if (y < image->rows)
		//failed = true;

		Mod* anMod = new Mod();

		anMod->mName = width;
		anMod->mVersion = height;
		anMod->mBits = aBits;

		p_fclose(fp);
		return anMod;
	}

	p_fclose(fp);

	return NULL;
}

typedef struct my_error_mgr * my_error_ptr;


bool ModLib::WriteDLLMod(const std::string& theFileName, Mod* theMod)
{
#ifdef ANDROID
    // 加载 DLL 文件
    HMODULE hModule = LoadLibrary(theFileName.c_str());
    if (!hModule) {
        return false; // 加载失败
    }

    // 假设 DLL 中有一个函数，可以获取 Mod 的相关信息
    typedef void (*GetModInfoFunc)(Mod*);
    GetModInfoFunc getModInfo = (GetModInfoFunc)GetProcAddress(hModule, "GetModInfo");

    if (getModInfo) {
        getModInfo(theMod); // 调用 DLL 中的函数来获取 Mod 的信息
    } else {
        FreeLibrary(hModule);
        return false; // 获取函数失败
    }

    // 这里假设 DLL 还提供了一个释放资源的函数
    typedef void (*CleanupFunc)();
    CleanupFunc cleanup = (CleanupFunc)GetProcAddress(hModule, "Cleanup");

    if (cleanup) {
        cleanup(); // 调用 DLL 中的清理函数
    }

    // 卸载 DLL
    FreeLibrary(hModule);

    return true; // 成功读取模块信息
#endif
    return false; // 成功读取模块信息
}

bool ModLib::WriteSOMod(const std::string& theFileName, Mod* theMod)
{
#ifdef ANDROID
    // 加载共享库 (.so 文件)
    void* handle = dlopen(theFileName.c_str(), RTLD_LAZY);
    if (!handle) {
        return false; // 加载失败
    }

    // 假设共享库中有一个函数，用于获取 Mod 的相关信息
    typedef void (*GetModInfoFunc)(Mod*);
    GetModInfoFunc getModInfo = (GetModInfoFunc) dlsym(handle, "GetModInfo");

    if (getModInfo) {
        getModInfo(theMod); // 调用so中的函数来获取 Mod 的信息
    } else {
        dlclose(handle);
        return false; // 获取函数失败
    }

    // 这里假设共享库还提供了一个释放资源的函数
    typedef void (*CleanupFunc)();
    CleanupFunc cleanup = (CleanupFunc) dlsym(handle, "Cleanup");

    if (cleanup) {
        cleanup(); // 调用共享库中的清理函数
    }

    // 卸载共享库
    dlclose(handle);

    return true; // 成功读取模块信息
#endif
	return false;
}

int ModLib::gAlphaComposeColor = 0xFFFFFF;
bool ModLib::gLoadDLLs = true;
bool ModLib::gLoadSOs = true;

Mod* ModLib::GetMod(const std::string& theFilename, bool lookForAlphaMod)
{
    int aLastDotPos = theFilename.rfind('.');
    int aLastSlashPos = (int)theFilename.rfind('/');

    std::string anExt;
    std::string aFilename;

    if (aLastDotPos > aLastSlashPos)
    {
        anExt = theFilename.substr(aLastDotPos, theFilename.length() - aLastDotPos);
        aFilename = theFilename.substr(0, aLastDotPos);
    }
    else
        aFilename = theFilename;

    Mod* anMod = NULL;

    // 根据平台加载适当的模块
#if defined(WINDOWS)
    if ((anMod == NULL) && ((strcasecmp(anExt.c_str(), ".dll") == 0) || (anExt.length() == 0)))
        anMod = GetDLLMod(aFilename + ".dll");
#elif defined(ANDROID)
    if ((anMod == NULL) && ((strcasecmp(anExt.c_str(), ".so") == 0) || (anExt.length() == 0)))
		anMod = GetSOMod(aFilename + ".so");
#endif

    // 检查 alpha 图像
    Mod* anAlphaMod = NULL;
    if (lookForAlphaMod)
    {
        // 检查 _ModName
        anAlphaMod = GetMod(theFilename.substr(0, aLastSlashPos + 1) + "_" +
                            theFilename.substr(aLastSlashPos + 1, theFilename.length() - aLastSlashPos - 1), false);

        // 检查 ModName_
        if (anAlphaMod == NULL)
            anAlphaMod = GetMod(theFilename + "_", false);
    }


    return anMod;
}
