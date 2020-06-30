@echo off

javac com\mingjiekeji\convert\pdf2docx\MyTest.java
jar cvfm MyTest.jar MANIFEST.MF com\mingjiekeji\convert\pdf2docx\MyTest.class
del /q com\mingjiekeji\convert\pdf2docx\MyTest.class
java -jar MyTest.jar
pause