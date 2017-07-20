CPP programe recieving and processing signal from WG26 IC/ID card reader.

此程序接收刷卡器传来的差分串行信号，提取出被刷卡片的卡号，并将卡号通过http协议post到本地的nodejs服务器上。
如果找不到nodejs服务器或者请求超时c程序会触发蜂鸣器2次，不开门。

nodejs服务器判断传来的卡号是否具有开门权限，如果开门会给刷卡器的/#/doors/opendoor/0发请求，否则给/#/doors/refuse/0发请求。