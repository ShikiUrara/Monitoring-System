#ifndef WEBPAGES_H
#define WEBPAGES_H

 const char HTML_SENSOR_CONFIG_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Sensor Configuration</title>
  <link rel="stylesheet" href="style.css">

</head>

<body>
  <nav>{navbar}</nav>
  <h2>Trang cấu hình cảm biến</h2>
  <form id="sensorForm" action="/sensorconfig" method="post">
    <label for="idNode">ID Node:</label>
    <input type="text" id="idNode" name="idNode" required>
    <label for="sensorType">Loại Sensor:</label>
    <select id="sensorType" name="sensorType" onchange="showFields()">
      <option value="Analog">Analog</option>
      <option value="RS485">RS485</option>
    </select>
    <br>
    <label for="actionType">Loại Hành Động:</label>
    <select id="actionType" name="actionType" onchange="showFields()">
      <option value="add">Add</option>
      <option value="edit">Edit</option>
      <option value="delete">Delete</option>
    </select>
    <br>
    <label for="idSensor">ID Sensor:</label>
    <input type="number" id="idSensor" name="idSensor" required>
    <label for="name">Tên:</label>
    <input type="text" id="name" name="name" required>
    <label for="txPin">TX Pin:</label>
    <input type="text" id="txPin" name="txPin">
    <label for="rxPin">RX Pin:</label>
    <input type="text" id="rxPin" name="rxPin">
    <label for="baudRate">Baud Rate:</label>
    <input type="text" id="baudRate" name="baudRate">
    <label for="analogPin">Analog Pin:</label>
    <input type="text" id="analogPin" name="analogPin">
    <label for="voltage">Voltage:</label>
    <input type="text" id="voltage" name="voltage">
    <label for="resolutionBit">Resolution Bit:</label>
    <input type="text" id="resolutionBit" name="resolutionBit">
    <input type="submit" value="Lưu">
  </form>
  <script>
    
    // JavaScript để xác định và tô màu nút khi trang được load
    document.addEventListener('DOMContentLoaded', function (analogPin) {
      showFields(); // Gọi hàm showFields() khi trang được load lần đầu
      // Lấy đường dẫn của trang hiện tại
      var currentPath = window.location.pathname;
      // Tìm nút tương ứng với đường dẫn hiện tại và thêm lớp active
      var navLinks = document.querySelectorAll('nav a');
      for (var i = 0; i < navLinks.length; i++) {
        if (navLinks[i].getAttribute('href') === currentPath) {
          navLinks[i].classList.add('active');
        }
      }
    });
    function showFields() {
      var sensorType = document.getElementById("sensorType").value;
      var actionType = document.getElementById("actionType").value;

      // Ẩn tất cả các trường và label
      var fields = ["txPin", "rxPin", "baudRate", "analogPin", "voltage", "resolutionBit"];
      fields.forEach(function (field) {
        var fieldElement = document.getElementById(field);
        var labelElement = document.querySelector('label[for="' + field + '"]');

        fieldElement.style.display = "none";
        labelElement.style.display = "none";
        fieldElement.removeAttribute("required"); // Bỏ bắt buộc

        // Nếu là trường Analog và đã chọn RS485, thì cũng bỏ focus
        if (sensorType !== "Analog" && fieldElement === document.activeElement) {
          fieldElement.blur();
        }
      });

      // Hiển thị các trường và label tùy thuộc vào loại action và loại sensor
      if (actionType === "add" || actionType === "edit") {
        if (sensorType === "Analog") {
          showFieldAndLabel("analogPin");
          showFieldAndLabel("voltage");
          showFieldAndLabel("resolutionBit");
        } else if (sensorType === "RS485") {
          showFieldAndLabel("txPin");
          showFieldAndLabel("rxPin");
          showFieldAndLabel("baudRate");
        }
      }

      // Kiểm tra điều kiện bắt buộc cho trường name và các trường thuộc loại sensor đã chọn
      var nameField = document.getElementById("name");
      var nameLabel = document.querySelector('label[for="name"]');

      var sensorFields = [];
      if (sensorType === "Analog") {
        sensorFields = ["analogPin", "voltage", "resolutionBit"];
      } else if (sensorType === "RS485") {
        sensorFields = ["txPin", "rxPin", "baudRate"];
      }

      if (actionType === "add") {
        // Bắt buộc các trường không rỗng nếu là action 'add'
        nameField.required = true;
        sensorFields.forEach(function (field) {
          var fieldElement = document.getElementById(field);
          fieldElement.required = true;
        });
      } else if (actionType === "edit" || actionType === "delete") {
        // Chỉ bắt buộc trường name nếu là action 'edit' hoặc 'delete'
        nameField.required = true;
        sensorFields.forEach(function (field) {
          var fieldElement = document.getElementById(field);
          fieldElement.required = false;
        });
      }
    }

    function showFieldAndLabel(field) {
      var fieldElement = document.getElementById(field);
      var labelElement = document.querySelector('label[for="' + field + '"]');

      fieldElement.style.display = "block";
      labelElement.style.display = "block";
      fieldElement.required = true; // Bắt buộc
    }

  </script>
</body>

</html>
)=====";
 // Khai báo chuỗi HTML trong bộ nhớ flash cho trang 404
    const char HTML_404_PAGE[] PROGMEM = R"(
      <html>
        <head>
          <meta charset='UTF-8'>
          <title>404 Not Found</title>
          <link rel='stylesheet' type='text/css' href='/style.css'>
        </head>
        <body>
          <h2>404 Not Found</h2>
          <p>Trang bạn đang tìm kiếm không tồn tại.</p>
        </body>
      </html>
    )";

    // Khai báo chuỗi HTML trong bộ nhớ flash cho trang đăng nhập
    const char HTML_LOGIN_PAGE[] PROGMEM = R"(
        <html>
        <head>
          <meta charset='UTF-8'>
          <link rel='stylesheet' type='text/css' href='/style.css'>
        </head>
        <body>
          <form action='/login' method='post' onsubmit='return handleLogin()'>
            <h2>Trang đăng nhập</h2>
            Tên người dùng: <input type='text' name='username'><br>
            Mật khẩu: <input type='password' name='password'><br>
            <input type='submit' value='Đăng nhập'>
          </form>
          <script>
            function handleLogin() {
              var username = document.getElementsByName('username')[0].value;
              var password = document.getElementsByName('password')[0].value;

              // Kiểm tra thông tin đăng nhập
              if (username === "admin" && password === "1234") {
                return true; // Cho phép submit form
              } else {
                alert("Đăng nhập không thành công");
                return false; // Ngăn chặn submit form
              }
            }
          </script>
        </body>
        </html>
    )";

    // Khai báo chuỗi HTML trong bộ nhớ flash cho trang kiểm soát
    const char HTML_CONFIG_PAGE[] PROGMEM = R"(
        <html>

        <head>
            <meta charset='UTF-8'>
            <link href="style.css" rel="stylesheet">
        </head>

        <body>
            <nav>{navbar}</nav>
            <h2>Trang cấu hình</h2>
            <form id='configForm' action='/config' method='post'>
                <h3>Cấu hình Wifi </h3>
                <label class="container">Wifi STA
                    <input type='checkbox' id='wifiCheckbox' name='wifi' onchange='showWiFiSettings()'>
                    <span class="checkmark"></span>
                </label>
                <div id='wifiSettings' style='display: none;'>
                    SSID: <div id='wifiLoading' style='display: none;'>Đang quét...</div>
                    <select name='wifiSSID' id='wifiSSID'></select><br>
                    Password: <input type='password' name='wifiPassword' id='wifiPassword'><br>
                    <input type='button' id='connect' value='Kết nối' onclick='connectWiFi()'>
                </div>

                <h3>Cấu hình MQTT</h3>
                <label class="container">MQTT
                    <input type='checkbox' id='mqttCheckbox' name='mqttCheckbox' onchange='showMQTTForm()'>
                    <span class="checkmark"></span>
                </label>
                <div id='mqttForm' style='display: none;'>
                    Server: <input type='text' name='mqttServer' id='mqttServer'><br>
                    Username: <input type='text' name='mqttUsername' id='mqttUsername'><br>
                    Password: <input type='password' name='mqttPassword' id='mqttPassword'><br>
                    Port: <input type='number' name='mqttPort' id='mqttPort'><br>
                </div>

                <h3>Cấu hình WiFi AP</h3>
                <label class="container">Wifi AP
                    <input type='checkbox' id='wifiAPCheckbox' name='wifiAPCheckbox' onchange='showWiFiAPForm()'>
                    <span class="checkmark"></span>
                </label>
                <div id='wifiAPForm' style='display: none;'>
                    SSID: <input type='text' name='wifiAPSSID' id='wifiAPSSID'><br>
                    Password: <input type='password' name='wifiAPPassword' id='wifiAPPassword'><br>
                </div>

                <input type='submit' value='Lưu'>
            </form>
            <script>
              // JavaScript để xác định và tô màu nút khi trang được load
              document.addEventListener('DOMContentLoaded', function () {
                // Lấy đường dẫn của trang hiện tại
                var currentPath = window.location.pathname;

                // Tìm nút tương ứng với đường dẫn hiện tại và thêm lớp active
                var navLinks = document.querySelectorAll('nav a');
                for (var i = 0; i < navLinks.length; i++) {
                  if (navLinks[i].getAttribute('href') === currentPath) {
                    navLinks[i].classList.add('active');
                  }
                }
              });
                function showMQTTForm() {
                    var mqttCheckbox = document.getElementById('mqttCheckbox');
                    var mqttForm = document.getElementById('mqttForm');
                    mqttForm.style.display = mqttCheckbox.checked ? 'block' : 'none';
                }

                function showWiFiAPForm() {
                    var wifiAPCheckbox = document.getElementById('wifiAPCheckbox');
                    var wifiAPForm = document.getElementById('wifiAPForm');
                    wifiAPForm.style.display = wifiAPCheckbox.checked ? 'block' : 'none';
                }
                function showWiFiSettings() {
                    var wifiCheckbox = document.getElementById('wifiCheckbox');
                    var wifiSettings = document.getElementById('wifiSettings');

                    if (wifiCheckbox.checked) {
                        wifiSettings.style.display = 'block';
                        loadAvailableWiFi();
                    } else {
                        wifiSettings.style.display = 'none';
                    }
                }

                function loadAvailableWiFi() {
                    var wifiSelect = document.getElementById('wifiSSID');
                    var wifiLoading = document.getElementById('wifiLoading');
                    // Ẩn danh sách SSID và hiển thị chữ "đang quét"
                    wifiSelect.style.display = 'none';
                    wifiLoading.style.display = 'block';
                    document.getElementById('connect').disabled = true;

                    // AJAX request để lấy danh sách WiFi
                    var xhr = new XMLHttpRequest();
                    xhr.onreadystatechange = function () {
                        if (xhr.readyState === XMLHttpRequest.DONE) {
                            if (xhr.status === 200) {
                                var wifiList = JSON.parse(xhr.responseText);
                                wifiSelect.innerHTML = ''; // Xóa tất cả các tùy chọn cũ

                                wifiList.forEach(function (wifi) {
                                  var option = document.createElement('option');
                                  option.value = wifi.ssid;
                                  option.text = wifi.ssid;
                                  var rssi = wifi.rssi;
                                  var signalStrength;
                                  if (rssi >= -50) {
                                    signalStrength = "Tuyệt vời";
                                  } else if (rssi >= -60) {
                                    signalStrength = "Mạnh";
                                  } else if (rssi >= -70) {
                                    signalStrength = "Trung bình";
                                  } else {
                                    signalStrength = "Yếu";
                                  }
                                  option.text = wifi.ssid + " - Cường độ: " + signalStrength;
                                  wifiSelect.appendChild(option);
                                });
                                // Hiển thị danh sách SSID và ẩn chữ "đang quét"
                                wifiSelect.style.display = 'block';
                                wifiLoading.style.display = 'none';
                                document.getElementById('connect').disabled = false;
                            }
                        }
                    };

                    xhr.open('GET', '/getWiFiList', true);
                    xhr.send();
                }
                function connectWiFi() {
                    var wifiSSID = document.getElementById('wifiSSID').value;
                    var wifiPassword = document.getElementById('wifiPassword').value;

                    // Kiểm tra độ dài mật khẩu
                    if (wifiPassword.length >= 8) {
                        // Gửi thông tin WiFi đến ESP8266 để kết nối
                        var xhr = new XMLHttpRequest();
                        xhr.onreadystatechange = function () {
                            if (xhr.readyState === XMLHttpRequest.DONE) {
                                if (xhr.status === 200) {
                                    alert('Kết nối WiFi thành công');
                                } else {
                                    alert('Kết nối WiFi thất bại');
                                }
                                document.getElementById('connect').disabled = false;
                            }
                        };

                        xhr.open('POST', '/connectWiFi', true);
                        xhr.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
                        xhr.send('ssid=' + encodeURIComponent(wifiSSID) + '&password=' + encodeURIComponent(wifiPassword));

                        // Thêm dòng này để ngăn chặn người dùng nhấn nút liên tục
                        document.getElementById('connect').disabled = true;
                    } else {
                        alert('Mật khẩu WiFi phải có ít nhất 8 ký tự');
                    }
                }
            </script>
        </body>

        </html>
    )";
#endif  // WEBPAGES_H
