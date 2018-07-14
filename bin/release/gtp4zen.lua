-- 如果不明白，可以删除这个文件！！！！！！删除后更好用：）
-- 写过程序的应该容易理解

-- in:   cur_move_num当前手数
-- in:   time_left剩余时间
-- out:  当前手参考时间
-- 说明：当这个函数存在时命令行的-T参数自动失效，单位全部是毫秒
function genmove_calctime(cur_move_num, time_left)
	if time_left > 240000 then		-- 剩余时间在240秒以上
		if cur_move_num < 50  then
			return 15000
		end
		if cur_move_num < 100  then
			return 12000
		end
		return 10000
	elseif time_left > 120000 then		-- 剩余时间在120-240秒之间
		return 8000
	elseif time_left > 60000 then		-- 剩余时间在60-120秒之间
		return 2000
	else					-- 剩余时间在60秒以内
		return 500
	end
end


-- 返回贴目，当这个函数存在时gtp的komi指令自动失效
-- 这个函数也可以删掉
function komi()
	return 7.5
end
