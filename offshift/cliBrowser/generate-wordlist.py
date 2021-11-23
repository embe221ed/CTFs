from string import printable

fp = open("wordlist.txt", "w")

for i in (printable):
  for j in (printable):
    for k in (printable):
      for l in (printable):
        for m in (printable):
            for n in (printable):
                for o in (printable):
                    fp.write(i + j + k + l + m + n + o + "\n");

fp.close()
