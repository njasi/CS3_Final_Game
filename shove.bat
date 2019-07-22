@echo off
git add -A
echo What should the commit message be?
set /p mess=
git commit -m "%mess%"
git push
@echo on