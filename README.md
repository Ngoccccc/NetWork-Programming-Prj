## How to run this project
- Git clone this project
- Change directory to cloned folder
- Install library ```ncurses.h``` with command ```sudo apt-get install libncurses5-dev```
- Run `make`
- Run `./server` để khởi động server
- Run `./client 127.0.0.1` để tạo ra 1 client mới thao tác với server

Thi thoảng server bị đơ (ít khi xảy ra) do hiện tượng `interleaved` khi cả server và client đều tạo ra các luồng sử dụng chung `stdout`, đặc điểm nhận dạng là đôi khi client in ra thiếu 1 dòng cuối cùng khi hiển thị các menu, nếu gặp lỗi này xin vui lòng khởi động lại server cũng như client.

Khi bắt đầu game đấu màn hình sẽ reset về giao diện game dẫn đến đen màn hình. Bạn chỉ cần lăn chuột lên hoặc nhấn nút mũi tên lên trên trong bàn phím thì sẽ chơi được game bình thường

Cập nhật so với checkpoint cuối:
- Đã có dừng trò chơi (pause) và gửi thông tin về trạng thái dừng/chơi cho người chơi còn lại
- Cập nhật về cách lưu điểm và hiển thị điểm (leaderboard) theo đúng logic
- Cập nhật thêm chức năng quản lí tài khoản : đổi mật khẩu .
