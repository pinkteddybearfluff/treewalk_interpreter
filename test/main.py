import subprocess

res = subprocess.run(["/home/wcosmo/Desktop/Projects/interpreter/cmake-build-debug/laven","../test/test.som"],capture_output=True,text=True)
print(res.stdout)
print(res.stderr)
