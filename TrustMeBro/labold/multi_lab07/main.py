import matplotlib.pyplot as plt
from skimage import color, exposure, io, util
from PIL import Image, ImageDraw, ImageFont

# ==========================================
# 1. ตั้งค่าตัวแปร (แก้ไขข้อมูลตรงนี้ให้เป็นของคุณเอง)
# ==========================================
image_path = "./org.png" # ชื่อไฟล์ภาพต้นฉบับของคุณ
student_name = "Napon_Narkphan"       # เปลี่ยนเป็นชื่อตัวเอง
student_id = "68070077"          # เปลี่ยนเป็นรหัสนักศึกษาตัวเอง

# โจทย์ข้อ 2: ให้ทดลอง kernel_size 3 ค่า (เช่น 32, 64, 128) 
# ลองเปลี่ยนตัวเลขตรงนี้ รันดูผลลัพธ์ แล้วเลือกค่าที่ภาพออกมาดีที่สุด
best_kernel_size = 256         

# ==========================================
# 2. โหลดภาพและทำ CLAHE 
# ==========================================
print("กำลังประมวลผลภาพ...")
# โหลดภาพสี
img = io.imread(image_path)

# ==========================================
# *** เพิ่มโค้ด 2 บรรทัดนี้เพื่อแก้ปัญหา RGBA ***
if img.shape[-1] == 4:
    img = img[:, :, :3] # ตัดช่อง Alpha ทิ้งไป ให้เหลือแค่ R, G, B
# ==========================================

# แปลงจาก RGB เป็น HSV (ได้ค่าเป็น float 0.0 - 1.0)
hsv_img = color.rgb2hsv(img)

# ทำ CLAHE เฉพาะช่องที่ 3 (Index 2 คือ Value/Brightness)
hsv_img[:, :, 2] = exposure.equalize_adapthist(hsv_img[:, :, 2], kernel_size=best_kernel_size)

# แปลงกลับจาก HSV เป็น RGB
output_rgb_float = color.hsv2rgb(hsv_img)

# ภาพที่ได้จะเป็น float ต้องแปลงเป็น uint8 (0-255) เพื่อให้ PIL นำไปวาดข้อความต่อได้
output_rgb_uint8 = util.img_as_ubyte(output_rgb_float)

# ==========================================
# 3. เพิ่มข้อความลงในภาพ (ด้วย PIL)
# ==========================================
# แปลง Numpy Array เป็น PIL Image
img_pil = Image.fromarray(output_rgb_uint8)
draw = ImageDraw.Draw(img_pil)

# เตรียมข้อความ
text_content = f"Name: {student_name}, Student ID: {student_id}, kernel size= {best_kernel_size}"
text_color = (255, 255, 0) # กำหนดเป็นสีเหลืองเพื่อให้ตัดกับภาพ (เปลี่ยนเป็น (255, 255, 255) ถ้าอยากได้สีขาว)

# โจทย์ข้อ 3: ปรับตำแหน่งข้อความให้อยู่บริเวณด้านล่างของภาพ
width, height = img_pil.size
text_position = (50, height - 60) # ขยับจากขอบล่างขึ้นมา 60 พิกเซล

# หากตัวหนังสือเล็กไป สามารถโหลดฟอนต์มาใช้ได้ (เอาเครื่องหมาย # ออกถ้ามีไฟล์ฟอนต์)
# try:
#     custom_font = ImageFont.truetype("arial.ttf", size=36)
#     draw.text(text_position, text_content, fill=text_color, font=custom_font)
# except:
#     draw.text(text_position, text_content, fill=text_color)

# วาดข้อความ (ใช้ฟอนต์เริ่มต้นของระบบ)
draw.text(text_position, text_content, fill=text_color)

# ==========================================
# 4. แสดงผลและบันทึกภาพลงไฟล์
# ==========================================
# แสดงภาพบนหน้าจอ
plt.figure(figsize=(8, 6))
plt.imshow(img_pil)
plt.axis("off")
plt.title(f"Result with Kernel Size: {best_kernel_size}")
plt.show()

# บันทึกไฟล์ภาพตาม Format ที่โจทย์กำหนด (เช่น 68xxxx_kernel_64.jpg)
output_filename = f"{student_id}_kernel_{best_kernel_size}.jpg"
img_pil.save(output_filename)
print(f"บันทึกไฟล์ภาพสำเร็จ: {output_filename}")