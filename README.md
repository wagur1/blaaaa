Phát triển Ứng dụng
Chợ Đồ Cũ Thứ Hai
Hiểu rõ yêu cầu và API thông qua mô tả chi tiết + bảng trạng thái trực quan

Mô tả Bài Toán
Bạn được yêu cầu phát triển một ứng dụng chợ đồ cũ mới. Người bán có thể đăng ký sản phẩm để bán và người mua có thể mua sản phẩm trên ứng dụng.

Đối với Người bán
Đăng ký sản phẩm để bán trên ứng dụng. Mỗi sản phẩm được gắn từ 3 đến 5 thẻ (tags) khác nhau. Các thẻ này dùng để tìm kiếm sản phẩm.

Đối với Người mua
Tìm kiếm sản phẩm bằng cách cung cấp 3 thẻ. Hệ thống sẽ trả về sản phẩm rẻ nhất có đủ cả 3 thẻ. Sau khi mua, sản phẩm bị xóa khỏi thị trường.

Đặc điểm quan trọng về giá:
Giá của sản phẩm thay đổi khi có lệnh adjustPrice đối với các thẻ mà sản phẩm đang có. Tuy nhiên, sản phẩm được đăng ký sau khi giá thay đổi sẽ không bị ảnh hưởng bởi các thay đổi trước đó.

Các Hàm API Cần Triển Khai
Triển khai các hàm sau trong file User Code (C/C++ hoặc Java).

INIT
init(int N)
Gọi đầu mỗi test case
Khởi tạo ứng dụng. Đặt số lượng thẻ có sẵn là N. Xóa toàn bộ sản phẩm đã đăng ký trước đó.

Tham số:
N: int — Số lượng thẻ có sẵn trên ứng dụng (5 ≤ N ≤ 30)
addProduct(int mPrice, int tagNum, char tagName[][10])
Người bán đăng ký một sản phẩm mới với giá mPrice và danh sách các thẻ.

mPrice
1 ≤ mPrice ≤ 1.000.000
tagNum
3 ≤ tagNum ≤ 5 (số thẻ)
tagName
Mảng chuỗi, mỗi chuỗi 3-9 ký tự chữ thường
Tất cả tagName phải khác nhau. Sản phẩm mới được thêm với giá hiện tại, không bị ảnh hưởng bởi các thay đổi giá trước đó.
buyProduct(char tag1[], char tag2[], char tag3[])
Trả về int
Người mua tìm kiếm sản phẩm có đủ cả 3 thẻ được chỉ định. Mua sản phẩm rẻ nhất trong số đó và trả về giá. Sản phẩm sau khi mua sẽ bị xóa khỏi ứng dụng.

tag1, tag2, tag3 khác nhau
Độ dài 3-9 chữ thường
Số sản phẩm tìm thấy ≤ 1000 (đảm bảo)
Trả về: Giá sản phẩm được mua (nếu có), hoặc -1 nếu không tìm thấy sản phẩm phù hợp.
Nếu có nhiều sản phẩm cùng giá rẻ nhất → được đảm bảo chỉ có 1 sản phẩm rẻ nhất.
adjustPrice(char tag1[], int changePrice)
Thay đổi giá của tất cả sản phẩm hiện có trên ứng dụng mà có chứa thẻ tag1 bằng giá trị changePrice.

Quy tắc thay đổi giá
changePrice > 0 → Giá tăng
changePrice < 0 → Giá giảm
Giá sau thay đổi luôn > 0 (đảm bảo)
Lưu ý quan trọng:
• Chỉ ảnh hưởng sản phẩm đang tồn tại tại thời điểm gọi hàm
• Sản phẩm đăng ký sau sẽ có giá riêng, không bị ảnh hưởng bởi thay đổi trước
• Số sản phẩm bị thay đổi ≤ 3000 (đảm bảo)
Ví Dụ Minh Họa
Chuỗi gọi hàm mẫu từ đề bài

#	Hàm	Kết quả
1	init(5)	-
2	buy("white","black","red")	-1
3	add(50, {"red","white","black"})	-
4	add(35, {"blue","red","green","white"})	-
5	add(50, {"black","red","green"})	-
6	addProduct(45, {"red","black","white","green","blue"})	-
7	adjustPrice("blue", +7)	-
8	buy("white","black","red")	50
9	add(80, {"green","white","black"})	-
10	buy("red","white","black")	52
11	adjustPrice("blue", -4)	-
12	add(33, {"green","white","black","blue"})	-
13	buy("blue","red","white")	38
14	buy("red","blue","white")	-1
15	buy("green","white","black")	33
Trạng thái thị trường tại các mốc quan trọng (theo đúng đề bài)
Order 7 — After adjustPrice(“blue”, 7)
50
red, white, black
42 (+7)
blue, red, green, white
50
black, red, green
52 (+7)
red, black, white, green, blue
→ Bảng [Table 2] trong đề bài
Order 8 — After buyProduct(“white”, “black”, “red”)
42
blue, red, green, white
50
black, red, green
52
red, black, white, green, blue
→ Bảng [Table 3] trong đề bài
Order 12 — After addProduct(33, ...)
38
blue, red, green, white
50
black, red, green
80
green, white, black
33
green, white, black, blue
→ Bảng [Table 4] trong đề bài
Ràng Buộc & Giới Hạn
INIT
1 lần
Gọi ở đầu mỗi test case
addProduct
≤ 30.000
lần mỗi test case
buyProduct
≤ 15.000
lần mỗi test case
adjustPrice
≤ 5.000
lần mỗi test case
Sản phẩm tìm thấy
≤ 1.000
mỗi lần buyProduct
Sản phẩm adjust
≤ 3.000
mỗi lần adjustPrice
Thời gian chạy
3 giây
C++ / Java (25 test cases)
Bộ nhớ
256 MB
Heap + Static (Stack: 1MB)
Về thẻ (Tags)
• Chuỗi chữ thường, độ dài 3 - 9 ký tự
• Kết thúc bằng ký tự '\0'
• Số thẻ có sẵn: 5 ≤ N ≤ 30
• Mỗi sản phẩm có 3 ≤ tagNum ≤ 5 thẻ
Code Mẫu - Main & User Code
Chọn ngôn ngữ để xem code mẫu. Hai file Java bên dưới đã được tích hợp đầy đủ và có thể copy trực tiếp.

Ngôn ngữ:
C++
Java
Tải sample_input.txt
main.cpp

Copy
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <time.h>

extern void init(int N);
extern void addProduct(int mPrice, int tagNum, char tagName[][10]);
extern int buyProduct(char tag1[], char tag2[], char tag3[]);
extern void adjustPrice(char tag1[], int changePrice);

/////////////////////////////////////////////////////////////////////////

#define INIT	0
#define ADD		1
#define BUY		2
#define ADJ		3

static void mstrcpy(char dst[], const char src[]) {
	int c = 0;
	while ((dst[c] = src[c]) != '\0') ++c;
}
static int mstrcmp(const char str1[], const char str2[]) {
	int c = 0;
	while (str1[c] != '\0' && str1[c] == str2[c]) ++c;
	return str1[c] - str2[c];
}

static bool run()
{
	int N, cmd, ans, ret, tnum, price;
	char tag[5][10];

	int Q = 0;
	bool okay = false;

	ret = ans = 0;
	okay = false;

	scanf("%d", &Q);
	for (int i = 0; i < Q; ++i)
	{
		scanf("%d", &cmd);
		switch (cmd)
		{
		case INIT:
			scanf("%d", &N);
			init(N);
			okay = true;
			break;
		case ADD:
			scanf("%d %d", &price, &tnum);
			for (int m = 0; m < tnum; m++) {
				scanf("%s", tag[m]);
			}
			addProduct(price, tnum, tag);
			break;
		case BUY:
			scanf("%d", &ans);
			for (int m = 0; m < 3; m++) {
				scanf("%s", tag[m]);
			}
			ret = buyProduct(tag[0], tag[1], tag[2]);
			if (ans != ret) {
				okay = false;
			}
			break;
		case ADJ:
			scanf("%s %d", tag[0], &price);
			adjustPrice(tag[0], price);
			break;
		default:
			okay = false;
		}
	}

	return okay;
}

int main()
{
	setbuf(stdout, NULL);
	freopen("input.txt", "r", stdin);

	int T, MARK;
	scanf("%d %d", &T, &MARK);

	clock_t start = clock();

	for (int tc = 1; tc <= T; tc++)
	{
		int score = run() ? MARK : 0;
		printf("#%d %d\n", tc, score);
	}

	clock_t end = clock();
	double elapsed = (double)(end - start) *1000 / CLOCKS_PER_SEC;
	printf("\nTotal execution time: %.0f mseconds\n", elapsed);

	return 0;
}
user.cpp

Copy

void init(int N)
{
}

void addProduct(int mPrice, int tagNum, char tagName[][10])
{
}

int buyProduct(char tag1[], char tag2[], char tag3[])
{
	return 0;
}

void adjustPrice(char tag1[], int changePrice)
{
}
