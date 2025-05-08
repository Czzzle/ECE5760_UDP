import wave
import struct

# === 读取有符号WAV文件并将其转换为无符号WAV文件 ===
def signed_to_unsigned_wav(input_wav, output_wav):
    with wave.open(input_wav, 'rb') as in_wav:
        # 获取WAV文件的参数
        channels = in_wav.getnchannels()
        sample_width = in_wav.getsampwidth()  # 16-bit 是2字节
        frame_rate = in_wav.getframerate()
        frames = in_wav.getnframes()
        
        # 确保是16位WAV文件
        if sample_width != 2:
            raise ValueError("只支持16位WAV文件")

        # 读取原始样本数据
        raw_data = in_wav.readframes(frames)

        # 将有符号16位样本解包
        signed_samples = struct.unpack("<{}h".format(frames * channels), raw_data)
        
        # 将每个有符号样本转换为无符号样本 (范围: 0-65535)
        unsigned_samples = [sample + 32768 for sample in signed_samples]

        # 打开输出WAV文件进行写入
        with wave.open(output_wav, 'wb') as out_wav:
            # 设置新的WAV文件参数（无符号WAV是16位PCM数据，仍然是单声道或立体声）
            out_wav.setnchannels(channels)
            out_wav.setsampwidth(2)  # 每个样本2字节
            out_wav.setframerate(frame_rate)

            # 将无符号样本打包为字节数据
            unsigned_raw_data = struct.pack("<{}H".format(len(unsigned_samples)), *unsigned_samples)
            
            # 写入转换后的数据到新WAV文件
            out_wav.writeframes(unsigned_raw_data)
        
    print(f"转换完成,无符号WAV文件已保存到: {output_wav}")

# === 用法示例 ===
input_wav = "splash.wav"  # 输入的有符号WAV文件路径
output_wav = "splash_unsigned.wav"  # 输出的无符号WAV文件路径

# 调用函数进行转换
signed_to_unsigned_wav(input_wav, output_wav)
