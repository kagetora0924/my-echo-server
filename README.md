# echoサーバーの実装
## 実現する機能
サーバーは、クライアントからメッセージを受け取り、それをただそのまま返す
## 実装した部分
* サーバーは単一のクライアントからメッセージを受け取り、そのまま返す
## 未実装
* 複数クライアント対応
* IPv6に対応しているがIPv4に対応していないかも？
* graceful shutdown
* クライアント側プログラム