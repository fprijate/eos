create database EOS
go

use EOS
go

create procedure increment (@x int output) as 
	set @x += 1
go

create procedure reverse (@x varchar(max) output) as 
	set @x = reverse(@x)
go
