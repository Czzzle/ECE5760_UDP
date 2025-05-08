import wave
import struct

# === 读取WAV文件，将有符号16位样本转换为无符号16位样本 ===
def signed_to_unsigned(wav_file, output_file):
    with wave.open(wav_file, 'rb') as wav:
        channels = wav.getnchannels()
        sample_width = wav.getsampwidth()
        frames = wav.getnframes()

        # 读取WAV文件的原始样本数据
        raw = wav.readframes(frames)
        
        # 检查是否是16位样本
        if sample_width != 2:
            raise ValueError("只支持16位WAV文件")

        # 解包16位有符号样本
        signed_samples = struct.unpack("<{}h".format(frames * channels), raw)
        
        # 转换为无符号16位（范围从0到65535）
        unsigned_samples = [sample + 32768 for sample in signed_samples]

        # 将无符号数据保存到新文件中
        with open(output_file, 'w') as f:
            for sample in unsigned_samples:
                f.write(f"{sample}\n")
    
    print(f"转换完成！无符号数据已保存到：{output_file}")

# === 用法示例 ===
wav_file = "splash.wav"  # 输入的WAV文件路径
output_file = "unsigned_splash.wav"  # 输出的文本文件路径，包含无符号数据

# 调用函数进行转换
signed_to_unsigned(wav_file, output_file)