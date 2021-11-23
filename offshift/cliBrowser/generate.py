import jwt
import datetime


public_key = open('./jwtRS256.key.pub', 'r').read()
print(public_key)

data = {
    "exp": datetime.datetime.timestamp(datetime.datetime.now()+datetime.timedelta(hours=1)),
    "orig_iat": datetime.datetime.timestamp(datetime.datetime.now()),
    "id": "admin"

}
encoded = jwt.encode(data, public_key, algorithm="HS256")
print(encoded)
