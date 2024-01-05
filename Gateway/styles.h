#ifndef STYLES_H
#define STYLES_H

    // Khai báo chuỗi CSS trong bộ nhớ flash
    const char CSS_STYLES[] PROGMEM = R"(
    /* CSS cho thanh điều hướng */
nav {
  background-color: #333; /* Màu nền thanh điều hướng */
  overflow: hidden;
}

/* CSS cho nút trong thanh điều hướng */
nav a {
  float: left;
  display: block;
  color: white;
  text-align: center;
  padding: 14px 16px;
  text-decoration: none;
}

/* CSS cho nút được chọn */
nav a.active {
  background-color: #4CAF50; /* Màu nền khi được chọn */
  color: white; /* Màu chữ khi được chọn */
}

/* Clear floating after the navigation links */
nav:after {
  content: "";
  display: table;
  clear: both;
}
        body {font-family: Arial, sans-serif;padding: 20px;background-color: #f0f0f0;}
        h2 {color: #333;}
        h3 {color: #666;}
        form {background-color: #fff;padding: 20px;border-radius: 5px;box-shadow: 0px 0px 10px 0px rgba(0,0,0,0.1);width: 50%; /* Giảm kích thước form xuống 50% */margin: auto; /* Căn giữa form */}
        input[type='text'], input[type='password'],input[type='number'], select {width: 100%;padding: 10px;margin: 5px 0 15px 0;display: inline-block;border: 1px solid #ccc;border-radius: 4px;box-sizing: border-box;}
        input[type='submit'] {width: 100%;background-color: #4CAF50;color: white;padding: 14px 20px;margin: 8px 0;border: none;border-radius: 4px;cursor: pointer;}
        input[type='submit']:hover {background-color: #45a049;}
        /* The container */
        .container {display: block;position: relative;padding-left: 35px;margin-bottom: 12px;cursor: pointer;font-size: 18px;-webkit-user-select: none;-moz-user-select: none;-ms-user-select: none;user-select: none;}
        /* Hide the browser's default checkbox */
        .container input {position: absolute;opacity: 0;cursor: pointer;height: 0;width: 0;}
        /* Create a custom checkbox */
        .checkmark {position: absolute;top: 0;left: 0;height: 20px;width: 20px;background-color: #eee;}
        /* On mouse-over, add a grey background color */
        .container:hover input ~ .checkmark {background-color: #ccc;}
        /* When the checkbox is checked, add a blue background */
        .container input:checked ~ .checkmark {background-color: #2196F3;}
        /* Create the checkmark/indicator (hidden when not checked) */
        .checkmark:after {content: "";position: absolute;display: none;}
        /* Show the checkmark when checked */
        .container input:checked ~ .checkmark:after {display: block;}
        /* Style the checkmark/indicator */
        .container .checkmark:after {left: 6px;top: 4px;width: 5px;height: 7px;border: solid white;border-width: 0 3px 3px 0;-webkit-transform: rotate(45deg);-ms-transform: rotate(45deg);transform: rotate(45deg);}
    )";

#endif  // STYLES_H
