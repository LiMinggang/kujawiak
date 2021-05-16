"c:\Program Files (x86)\Poedit\GettextTools\bin\xgettext.exe" -f .\src.list -o .\kuview.pot -LC -k_

"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_zh_CN.po .\kuview.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_zh_TW.po .\kuview.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_ru.po .\kuview.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_ja.po .\kuview.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_it_IT.po .\kuview.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_fr.po .\kuview.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_de.po .\kuview.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_pl.po .\kuview.pot
"c:\Program Files (x86)\Poedit\GettextTools\bin\msgmerge.exe" -U .\kuview_uk.po .\kuview.pot
del .\*.pot
del .\*.po~