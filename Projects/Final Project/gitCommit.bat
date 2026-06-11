@echo off
git add -A
set /p commitMsg=Commit Message: 
git commit -m "%commitMsg%"
git push
echo:
echo All Changes Have Been Commited
echo:
git log -1 --stat
echo Closing in 5...
timeout /t 5 /nobreak > nul