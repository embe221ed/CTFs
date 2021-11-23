There are few options:

```bash
➜ help
help page :
- login
- hello
- ping
- flag
- exit
```

Function responsible for getting the options is `sym.main.get_input`

The function which we are interested in is `sym.main.get_flag`

The ip address is: `161.97.176.150:6666`

```bash
$ curl -X POST http://161.97.176.150:6666/login --data "username=test&password=test"
{"code":200,"expire":"2021-01-26T14:05:12+01:00","token":"eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJleHAiOjE2MTE2NjYzMTIsImlkIjoidGVzdCIsIm9yaWdfaWF0IjoxNjExNjYyNzEyfQ.ySFFPc0GtOpu1fyYBeYCu8uNcupTXF-0ROVakEaJfiI"}
```

- http://%s/ping
- http://%s/login
- http://%s/auth/flag
- http://%s/auth/hello
