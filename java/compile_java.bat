@echo off

javac MyTest.java
jar cvfm MyTest.jar MANIFEST.MF MyTest.class
del /q MyTest.class
java -jar MyTest.jar
pause