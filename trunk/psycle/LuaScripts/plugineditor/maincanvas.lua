-- psycle plugineditor (c) 2015, 2016 by psycledelics
-- File: maincanvas.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

-- require('mobdebug').start()
-- local serpent = require("psycle.serpent")

local machine = require("psycle.machine")
local machines = require("psycle.machines")
local catcher = require("psycle.plugincatcher")

local cfg = require("psycle.config"):new("PatternVisual")
local signal = require("psycle.signal")
local run = require("psycle.run")
local point = require("psycle.ui.point")
local dimension = require("psycle.ui.dimension")
local rect = require("psycle.ui.rect")
local boxspace = require("psycle.ui.boxspace")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local image = require("psycle.ui.image")
local item = require("psycle.ui.canvas.item")
local group = require("psycle.ui.canvas.group")
local canvas = require("psycle.ui.canvas")
local text = require("psycle.ui.canvas.text")
local button = require("psycle.ui.canvas.button")
local splitter = require("psycle.ui.canvas.splitter")
local combobox = require("psycle.ui.canvas.combobox")
local scintilla = require("psycle.ui.canvas.scintilla")
local lexer = require("psycle.ui.canvas.lexer")

local filehelper = require("psycle.file")
-- local fileobserver = require("psycle.fileobserver")
local fileopen = require("psycle.ui.fileopen")
local filesave = require("psycle.ui.filesave")
local fileexplorer = require("fileexplorer")

local toolicon = require("psycle.ui.canvas.toolicon")
local toolbar = require("psycle.ui.canvas.toolbar")
local tabgroup = require("psycle.ui.canvas.tabgroup")

local settings = require("settings")
local project = require("project")
local search = require("search")
local callstack = require("callstack")
local pluginselector = require("pluginselector")
local templateparser = require("templateparser")

local maincanvas = canvas:new()

function maincanvas:new(machine)
  local c = canvas:new()  
  setmetatable(c, self)
  self.__index = self  
  c.machine_ = machine  
  c:init()
  return c
end

function maincanvas:init()  
  self.machines = machines:new()
  -- self.fileobservers = {}  
  self:invalidatedirect()     
  self.togglecanvas = signal:new() 
  self:addornament(ornamentfactory:createfill(settings.canvas.colors.background))
  self:setupfiledialogs()
  self:inittoolbar()   
  self:createsearch()
  self:createcreateeditplugin()  
  self:createoutputs()
  splitter:new(self, splitter.HORZ)  
  self.fileexplorer = self:createfileexplorer()
  splitter:new(self, splitter.VERT)
  self:createpagegroup()  
end

function maincanvas:createcreateeditplugin()  
  self.pluginselector = pluginselector:new(self)
                                      :hide()
									  :setalign(item.ALTOP)
                                      :setautosize(false, false)
									  :setposition(rect:new(point:new(0, 0), dimension:new(0, 200)))  
  self.pluginselector.doopen:connect(maincanvas.onopenplugin, self)
  self.pluginselector.docreate:connect(maincanvas.oncreateplugin, self)
end

function maincanvas:createsearch()
  self.search = search:new(self)
                      :setposition(rect:new(point:new(0, 0), dimension:new(200, 200)))
					  :hide()
					  :setalign(item.ALBOTTOM)
  self.search.dosearch:connect(maincanvas.onsearch, self)
  self.search.doreplace:connect(maincanvas.onreplace, self)
  self.search.dohide:connect(maincanvas.onsearchhide, self)
  self.searchbeginpos = -1
  self.searchrestarted = false
end

function maincanvas:createoutputs()
  self.outputs = tabgroup:new(self)
                         :setposition(rect:new(point:new(0, 0), dimension:new(0, 180)))
						 :setalign(item.ALBOTTOM)
  self.output = self:createoutput()  
  self.outputs:addpage(self.output, "Output")  
  self.callstack = self:createcallstack()  
  self.outputs:addpage(self.callstack, "Call stack")  
end

function maincanvas:createoutput()
  local output = scintilla:new()
  output:setforegroundcolor(settings.sci.default.foreground)
  output:setbackgroundcolor(settings.sci.default.background) 
  output:styleclearall()
  output:setlinenumberforegroundcolor(0x939393)
  output:setlinenumberbackgroundcolor(0x232323)    
  output:setmarginbackgroundcolor(0x232323)  
  return output
end

function maincanvas:createcallstack()
  local callstack = callstack:new()
  callstack.change:connect(maincanvas.oncallstackchange, self)
  callstack:setbackgroundcolor(0x2F2F2F) --settings.canvas.colors.background)
  callstack:settextcolor(settings.canvas.colors.foreground)
  return callstack
end

function maincanvas:createpagegroup()
  self.pages = tabgroup:new(self):setalign(item.ALCLIENT):setautosize(false, false)
  self.pages.dopageclose:connect(maincanvas.onclosepage, self)
  self.newpagecounter = 1
end

function maincanvas:createpluginexplorer()
  local pluginexplorer = pluginexplorer:new(self)
                                       :setposition(rect:new(point:new(0, 0), dimension:new(200, 0)))
									   :setalign(item.ALLEFT)
  pluginexplorer:setbackgroundcolor(0x2F2F2F) --settings.canvas.colors.background)
  pluginexplorer:settextcolor(settings.canvas.colors.foreground)  
  pluginexplorer.click:connect(maincanvas.onpluginexplorerclick, self)
  pluginexplorer.onremove:connect(maincanvas.onpluginexplorernoderemove, self)
  return pluginexplorer
end

function maincanvas:createfileexplorer()
  local fileexplorer = fileexplorer:new(self)
                                   :setposition(rect:new(point:new(0, 0), dimension:new(200, 0)))
								   :setalign(item.ALLEFT)
  fileexplorer:setpath(cfg:luapath())  
  fileexplorer.click:connect(maincanvas.onpluginexplorerclick, self)
  fileexplorer.onselectmachine:connect(maincanvas.onselectmachine, self)
  --pluginexplorer.onremove:connect(maincanvas.onpluginexplorernoderemove, self)
  return fileexplorer
end

function maincanvas:setupfiledialogs()
  self.fileopen = fileopen:new()
  local that = self
  function self.fileopen:onok(fname) that:openfromfile(fname) end
  self.filesaveas = filesave:new() 
end

function maincanvas:setoutputtext(text)
  self.output:addtext(text)  
end

function maincanvas:setplugindir(plugininfo)  
  local path = plugininfo:dllname()
  path = path:sub(1, -5).."\\"
  self.fileexplorer:setpath(path)
  self.fileexplorer:setmachinename(plugininfo:name())
end

function maincanvas:dopageexist(fname)
  local found = nil
  local items = self.pages.children:items()    
  for i=1, #items do
    local page = items[i]
    if page:filename() == fname then
       found = page
       break
    end
  end
  return found
end

function maincanvas:setlexer(page)  
  page:setforegroundcolor(0xCACACA)
  page:setbackgroundcolor(settings.sci.default.background)   
  page:styleclearall()
  page:setlinenumberforegroundcolor(0x939393)
  page:setlinenumberbackgroundcolor(0x232323)
  page:setmarginbackgroundcolor(0x232323) 
  page:setselbackgroundcolor(0xFFFFFF)    
  page:setselalpha(75)
  local lex = lexer:new()
  lex:setkeywords(settings.sci.lexer.keywords)
  lex:setcommentcolor(settings.sci.lexer.commentcolor)
  lex:setcommentlinecolor(settings.sci.lexer.commentlinecolor)
  lex:setcommentdoccolor(settings.sci.lexer.commentdoccolor)
  lex:setidentifiercolor(settings.sci.lexer.identifiercolor)
  lex:setnumbercolor(settings.sci.lexer.numbercolor)
  lex:setoperatorcolor(settings.sci.lexer.operatorcolor)
  lex:setstringcolor(settings.sci.lexer.stringcolor)
  lex:setwordcolor(settings.sci.lexer.wordcolor)
  lex:setfoldingcolor(settings.sci.lexer.foldingcolor)
  page:setlexer(lex)         
  page:setfont(settings.sci.lexer.font.name, settings.sci.lexer.font.size)
  page:setcaretcolor(0x939393)
  page:setcaretlinebackgroundcolor(0x373533)
  page:showcaretline()
end

function maincanvas:createmodulepage()
  local page = self:createpage()  
  return page
end

function maincanvas:createpage()
  local page = scintilla:new():setautosize(false, false)
  local that = self
  function page:onkeydown(ev)
    if ev:ctrlkey() then
      if ev:keycode() == 70 or ev:keycode() == 83 then
        ev:preventdefault()
      end            
    end
  end    
  self:setlexer(page)    
  page:setmargin(boxspace:new(2, 0, 0, 0))
  return page
end

function maincanvas:onkeydown(ev)
  if ev:ctrlkey() then
    if ev:keycode() == 70 then
      self:displaysearch() 
      ev:stoppropagation()
    elseif ev:keycode() == 83 then      
      self:savepage()  
      ev:stoppropagation()
    elseif ev:keycode() == ev.F5  then
      self:playplugin()
      ev:stoppropagation()
    end
  end  
end

function maincanvas:openfromfile(fname, line)
  if not line  then line = 0 end
  local page = self:dopageexist(fname)  
  if page ~= nil then
    page:reload()
    if self.pages:activepage() ~= page then	  
      self.pages:setactivepage(page)
	end
  else    
    page = self:createpage()
    page:loadfile(fname)    
    local sep = "\\"  
    local dir = fname:match("(.*"..sep..")")    
    --self:addfiletowatcher(dir)	
    local name = fname:match("([^\\]+)$")           
    self.pages:addpage(page, name)         
  end  
  page:gotoline(line - 1)    
end

function maincanvas:setcallstack(trace)
  for i=1, #trace do    
    self.callstack:addline(trace[i])
  end  
end

function maincanvas:createnewpage()
  local page = self:createpage()
  self.pages:addpage(page, "new"..self.newpagecounter)
  page.pagecounter = self.newpagecounter
  self.newpagecounter = self.newpagecounter + 1
end

function maincanvas:savepage()
  local page = self.pages:activepage()
  if page then
    local fname = ""     
    if page:hasfile() then
	  fname = page:filename()    
	  local sep = "\\"  
      local dir = fname:match("(.*"..sep..")")    
--      local fileobserver = self:findfileobserverdir(dir)
--	  if fileobserver then
--	    fileobserver:stopwatching()
--	  end
      page:savefile(fname)
      fname = fname:match("([^\\]+)$")
      self.pages:setlabel(page, fname)
--	  if fileobserver then
--	    fileobserver:startwatching()
--	  end
    elseif self.filesaveas:show() then      
      fname = self.filesaveas:filename()          
      page:savefile(fname)
      fname = fname:match("([^\\]+)$")
      self.pages:setlabel(page, fname)
    end	
  end
end

function maincanvas:playplugin()  
  local pluginindex = psycle.proxy.project:pluginindex()  
  if pluginindex ~= -1 then
     machine = machine:new(pluginindex)
     if machine then
       self:savepage()
       machine:reload()    	   
	   self.playicon:seton(true)	   
     end
  else
    --self:savepage()    
    if  psycle.proxy.project:plugininfo() then
      local fname = psycle.proxy.project:plugininfo():dllname():match("([^\\]+)$"):sub(1, -5)    
      machine = machine:new(fname)      
      local pluginindex = self.machines:insert(machine)    
      psycle.proxy.project:setpluginindex(pluginindex)	  
      self:fillinstancecombobox()
      self:setpluginindex(pluginindex)
	  self.playicon:seton(true)	  
    end
  end  
end

function maincanvas:inittoolbar()  
  self.tg = group:new(self):setautosize(false, true):setalign(item.ALTOP):setmargin(boxspace:new(5))  
  self.windowtoolbar = self:initwindowtoolbar():setalign(item.ALRIGHT)  
  self:initfiletoolbar():setalign(item.ALLEFT)
  self:initplaytoolbar():setalign(item.ALLEFT):setmargin(boxspace:new(0, 0, 0, 20))  
  self:initstatus()
end

function maincanvas:initstatus()  
  local g = group:new(self.tg):setalign(item.ALRIGHT):setautosize(true, false)
  --g:addornament(ornamentfactory:createfill(0xFF0000))  
  self.searchrestartstatus = text:new(g)
                                 :settext("")
								 :setautosize(false, false)
								 :setposition(rect:new(point:new(0, 0), dimension:new(150, 0)))
								 :setalign(item.ALLEFT)
								 :setverticalalignment(item.ALCENTER)
								 :setmargin(boxspace:new(0, 0, 0, 5))
								 :addornament(ornamentfactory:createfill(settings.canvas.colors.background))
  self.modifiedstatus = text:new(g)
                            :settext("")
							:setautosize(false, false)
							:setposition(rect:new(point:new(0, 0), dimension:new(100, 0)))
							:setalign(item.ALLEFT)
							:setverticalalignment(item.ALCENTER)
							:setmargin(boxspace:new(0, 5, 0, 0))
							:addornament(ornamentfactory:createfill(settings.canvas.colors.background))
  text:new(g)
      :settext("LINE")
	  :setautosize(true, false)
	  :setalign(item.ALLEFT)
	  :setverticalalignment(item.ALCENTER)
	  :setmargin(boxspace:new(0, 5, 0, 0))
  self.linestatus = text:new(g)
                        :settext("1")
						:setautosize(false, false)
						:setposition(rect:new(point:new(0, 0), dimension:new(50, 0)))
						:setalign(item.ALLEFT)
						:setverticalalignment(item.ALCENTER)
						:setmargin(boxspace:new(0, 5, 0, 0))
						:addornament(ornamentfactory:createfill(settings.canvas.colors.background))
  text:new(g)
      :settext("COL")
	  :setautosize(true, false)
	  :setalign(item.ALLEFT)
	  :setverticalalignment(item.ALCENTER)
	  :setmargin(boxspace:new(0, 5, 0, 0))
  self.colstatus = text:new(g)
                       :settext("1")
					   :setautosize(false, false)
					   :setposition(rect:new(point:new(0, 0), dimension:new(50, 0)))
					   :setalign(item.ALLEFT)
					   :setverticalalignment(item.ALCENTER)
					   :setmargin(boxspace:new(0, 5, 0, 0))
					   :addornament(ornamentfactory:createfill(settings.canvas.colors.background))
  text:new(g)
      :settext("INSERT")
	  :setautosize(true, false)
	  :setalign(item.ALLEFT)
	  :setverticalalignment(item.ALCENTER)
	  :setmargin(boxspace:new(0, 5, 0, 0))
  self.insertstatus = text:new(g)
                          :settext("ON")
						  :setautosize(false, false)
						  :setposition(rect:new(point:new(0, 0), dimension:new(30, 0)))
						  :setverticalalignment(item.ALCENTER)
						  :setalign(item.ALLEFT)
						  :addornament(ornamentfactory:createfill(settings.canvas.colors.background))
end

function maincanvas:setwindowiconin()
  self.windowtoolbar.img = image:new()
                                :load(settings.picdir.."arrow_in.png")
                                :settransparent(0xFFFFFF)
  self.windowtoolbar:seton(false)
end

function maincanvas:setwindowiconout()
   self.windowtoolbar.img = image:new()
                                 :load(settings.picdir.."arrow_out.png")
                                 :settransparent(0xFFFFFF)
   self.windowtoolbar:seton(false)
end

function maincanvas:initwindowtoolbar()  
  local icon = toolicon:new(self.tg, settings.picdir.."arrow_out.png", 0xFFFFFF)  
  local that = self
  function icon:onclick()   
    that.togglecanvas:emit()
  end
  return icon
end

function maincanvas:onselectmachine()
  local catcher = catcher:new()
  local infos = catcher:infos()
  self.pluginselector:show()
  self:updatealign()    
end

function maincanvas:fillinstancecombobox()
  local items = {"new instance"}
  self.cbxtopluginindex = {-1}
  if (psycle.proxy.project:plugininfo()) then
    for machineindex= 0, 255 do
      local machine = machine:new(machineindex);       
      if machine and machine:type() == machine.MACH_LUA and machine:pluginname() == psycle.proxy.project:plugininfo():name() then
        items[#items + 1] = machine:pluginname().."["..machineindex.."]"
        self.cbxtopluginindex[#self.cbxtopluginindex + 1] = machineindex
      end
    end     
  end
  self.cbx:setitems(items)
  self.cbx:setitemindex(1)
  return self
end  

function maincanvas:setpluginindex(pluginindex)
  local cbxindex = 1  
  for i = 1, #self.cbxtopluginindex do    
    if self.cbxtopluginindex[i] == pluginindex then       
       cbxindex = i
       break    
    end
  end  
  self.cbx:setitemindex(cbxindex)  
end

function maincanvas:createinstanceselect(parent)  
  self.cbx = combobox:new(parent)
                     :setautosize(false, false)
                     :setposition(rect:new(point:new(0, 0), dimension:new(200, 200)))
					 :setalign(item.ALLEFT)
  local that = self
  function self.cbx:onselect()    
    local pluginindex = that.cbxtopluginindex[self:itemindex()]
    if pluginindex then
      psycle.proxy.project:setpluginindex(pluginindex)       
    else
      psycle.proxy.project:setpluginindex(-1)       
    end
  end
  self.cbx:setitems({"new instance"})
  self.cbx:setitemindex(1)
end

function maincanvas:initfiletoolbar()  
  local t = toolbar:new(self.tg)
  local inew = toolicon:new(t, settings.picdir.."new.png", 0xFFFFFF)
  local iopen = toolicon:new(t, settings.picdir.."open.png", 0xFFFFFF)  
  local isave = toolicon:new(t, settings.picdir.."save.png", 0xFFFFFF)  
  local iundo = toolicon:new(t, settings.picdir.."undo.png", 0xFFFFFF)
  local iredo = toolicon:new(t, settings.picdir.."redo.png", 0xFFFFFF)  
  local that = self    
  function inew:onclick() 
    that:createnewpage()    
  end  
  function iopen:onclick() that.fileopen:show() end
  function isave:onclick() that:savepage() end
  function iredo:onclick() that:redo() end
  function iundo:onclick() that:undo() end
  return t
end

function maincanvas:initplaytoolbar()  
  local t = toolbar:new(self.tg)  
  self.playicon = toolicon:new(t, settings.picdir.."poweroff.png", 0xFFFFFF)  
  self.playicon.is_toggle = true
  local on = image:new():load(settings.picdir.."poweron.png")
  on:settransparent(0xFFFFFF)
  self.playicon:settoggleimage(on)    
  local that = self
  function self.playicon:onclick()
    that:playplugin()
  end
  self:createinstanceselect(t)
  return t
end

function maincanvas:oncallstackchange(info)
  self:openinfo(info)
end

function maincanvas:openinfo(info)
  local isfile = false
  if info.source:len() > 1 then
     local firstchar = info.source:sub(1, 1)     
     if firstchar == "@" then
       local fname = info.source:sub(2) 
       self:openfromfile(fname, info.line)
       isfile = true
     end
  end
  return isfile
end

function maincanvas:findprevsearch(page, pos)
  local cpmin, cpmax = 0, 0
  cpmin = pos
  cpmax = 0
  if page:hasselection() then 
    cpmax = cpmax - 1
  end
  return cpmin, cpmax
end

function maincanvas:findnextsearch(page, pos)
  local cpmin, cpmax = 0, 0
  cpmin = pos
  cpmax = page:length()
  if page:hasselection() then       
    cpmin = cpmin + 1
  end
  return cpmin, cpmax
end

function maincanvas:findsearch(page, dir, pos)
  local cpmin, cpmax = 0, 0
  if dir == search.DOWN then      
    cpmin, cpmax = self:findnextsearch(page, pos)
  else
    cpmin, cpmax = self:findprevsearch(page, pos)
  end
  return cpmin, cpmax
end

function maincanvas:onsearch(searchtext, dir, case, wholeword, regexp)
  local page = self.pages:activepage()
  if page then 
    page:setfindmatchcase(case):setfindwholeword(wholeword):setfindregexp(regexp)      
    local cpmin, cpmax = self:findsearch(page, dir, page:selectionstart())
    local line, cpselstart, cpselend = page:findtext(searchtext, cpmin, cpmax)
    if line == -1 then      
      if dir == search.DOWN then
        page:setsel(0, 0)
        local cpmin, cpmax = self:findsearch(page, dir, 0)
        line, cpselstart, cpselend = page:findtext(searchtext, cpmin, cpmax)        
      else
        page:setsel(0, 0)
        local cpmin, cpmax = self:findsearch(page, dir, page:length())
        line, cpselstart, cpselend = page:findtext(searchtext, cpmin, cpmax)
      end             
    end
    if line ~= -1 then
      page:setsel(cpselstart, cpselend)
      if self.searchbeginpos == cpselstart then
        self.searchbegin = -1        
        self.searchrestart = true
      else
        self.searchrestart = false
      end
      if self.searchbeginpos == -1 then
        self.searchbeginpos = cpselstart        
      end
    end    
  end
end

function maincanvas:onreplace()
  local page = self.pages:activepage()
  if page then 
    if page:hasselection() then     
      page:replacesel(self.search.replacefield:text())      
    end                 
    self:onsearch(self.search.edit:text(), 
                  self.search.DOWN,
                  self.search.casesensitive:check(),
                  self.search.wholeword:check(),
                  self.search.useregexp:check())
  end
end

function maincanvas:displaysearch(ev)
  self.search:show():onfocus()
  self:updatealign()  
end

function maincanvas:oncreateplugin(template, pluginname)
  local filters = template.filters 
  for i=1, #filters do	
    local fname = string.sub(filters[i].outputpath, 1, -5)    
    self:openplugin(pluginname.."\\"..fname, pluginname)
  end 					  
  
  local catcher = catcher:new():rescannew()
  --local infos = catcher:infos()        
  --for i=1, #infos do       
   -- if infos[i]:type() == machine.MACH_LUA and
   --    infos[i]:name():lower() == pluginname:lower() then      
    --  psycle.proxy.project = project:new():setplugininfo(infos[i])
    --  break;     
  --  end
  --end    
  self.pluginselector:hide()
  self:updatealign()
end

function maincanvas:onopenplugin(pluginpath, pluginname, info)  
  self:openplugin(pluginpath, pluginname, info)
  psycle.proxy.project = project:new():setplugininfo(info)
  self:fillinstancecombobox()  
  self.machine_:settitle(pluginname)
end

function findlast(haystack, needle)
  local i=haystack:match(".*"..needle.."()")
  if i==nil then return else return i end
end

function maincanvas:openplugin(pluginpath, pluginname, plugininfo)  
  self:preventfls()
  self:closealltabs()      
  self.fileexplorer:sethomepath(cfg:luapath().."\\"..pluginpath:sub(1, pluginpath:find("\\") - 1))
  self.fileexplorer:setmachinename(pluginname)
  self.fileexplorer.machineselector:settextcolor(0xFFFFFF)
							       :setfont({name="arial", height=13, style=1})  
  self:openfromfile(cfg:luapath().."\\"..pluginpath..".lua")    
  self:updatealign()
  self:enablefls()  
  self:invalidate()
end

function maincanvas:closealltabs()    
  self.pages:removeall()
  self.newpagecounter = 1
end

function maincanvas:onpluginexplorerclick(ev)
   if ev.filename ~= "" and ev.path then       
     self:openfromfile(ev.path.."\\"..ev.filename, 0)
   end
end

function maincanvas:onpluginexplorernoderemove(node)  
  node:parent():remove(node)
  self.pluginexplorer:updatetree()
  local items = self.pages.children:items()
  for i = 1, #items do        
    if node.path..node.filename == items[i]:filename() then
      self.pages:removepage(items[i])
      filehelper.remove(node.path..node.filename)
      break;
    end
  end  
end

function maincanvas:onidle()  
  if self.pages:activepage() then
    local l = (self.pages:activepage():line() + 1)..""
    local c = (self.pages:activepage():column() + 1)..""
    local ovr = "ON"
    if  self.pages:activepage():ovrtype() then
      ovr = "OFF"
    else
      ovr = "ON"
    end    
    if self.linestatus:text() ~= l then
      self.linestatus:settext(l)      
    end
    if self.colstatus:text() ~= c then
      self.colstatus:settext(c)      
    end
    if self.insertstatus:text() ~= ovr then
      self.insertstatus:settext(ovr)      
    end
    if self.searchrestart then
      searchrestartstr = "SEARCH AT BEGINNING POINT"
    else
      searchrestartstr = ""
    end
    if self.searchrestartstatus:text() ~= searchrestartstr then
      self.searchrestartstatus:settext(searchrestartstr)
    end
    local modified = "O"
    if  self.pages:activepage():modified() then
      modified = "MODIFIED"
    else
      modified = ""
    end    
    if self.modifiedstatus:text() ~= modified then
       self.modifiedstatus:settext(modified)
    end
  end
  self:updatepowericon()
end

function maincanvas:updatepowericon()
  if self.cbxtopluginindex ~= nil then    
    local isselectedmachinemute = self.machines:muted(self.cbxtopluginindex[self.cbx:itemindex()])	
    if self.playicon:on() == isselectedmachinemute then      
	  self.playicon:toggle()
    end
  end  
end

function maincanvas:onsearchhide()
  self.search:hide()
  self:updatealign()
  if self.pages:activepage() then
    self.pages:activepage():setfocus()
  end
end

function maincanvas:onclosepage(ev)  
  if ev.page:modified() and psycle.confirm("Do you want to save changes to "..ev.page:filename().."?") then
    self:savepage()
  end
end

function maincanvas:undo()  
  if self.pages:activepage() then
    self.pages:activepage():undo()
  end
end

function maincanvas:redo()
  if self.pages:activepage() then
    self.pages:activepage():redo()
  end
end

--function maincanvas:findfileobserverdir(dir)
--  local result = nil
--  for i=1, #self.fileobservers do
--    if self.fileobservers[i]:directory() == dir then
--	  result = self.fileobservers[i]
--	  break
--	end  
--  end  
--  return result
--end

-- function maincanvas:addfiletowatcher(dir)  
--  if not self:findfileobserverdir(dir) then
--    local fileobserver = fileobserver:new()
--	fileobserver:setdirectory(dir)
--  self.fileobservers[#self.fileobservers + 1] = fileobserver
--	local that = self
--	function fileobserver:oncreatefile(path)	
--	end	
--	function fileobserver:onchangefile(path)
--    local pages = that.pages.children:items()
--    for i=1, #pages do 
--	    if pages[i]:filename() == path then        	 
--	      if psycle.confirm("File "..path.." outside changed. Do you want to reload ?") then
--	        that:openfromfile(path)
--			break;
--	      end
--	    end	    
--      end    
--  end  
--  function fileobserver:ondeletefile(path)
--	  local pages = that.pages.children:items()     
--    for i=1, #pages do 	  
--	    if pages[i]:filename() == path then        	 
--	      if not psycle.confirm('The file "'..path..'" '.."doesn't exist anymore.\nKeep this file in editor?") then
--           that.pages:removepage(pages[i])
--			 local sep = "\\"  
--           local filedir = path:match("(.*"..sep..")")    
--			 that:removefilewatcher(filedir)
--           break;
--	      end
--	    end
--	  end
--  end
--	fileobserver:startwatching()		
-- end    
--end

--function maincanvas:removefilewatcher(dir)    
--  local found = false
--  for i=1, #self.pages do
--    local fname = self.pages[i]:fname()
--    local sep = "\\"  
--    local filedir = fname:match("(.*"..sep..")")    	
--	if filedir == dir then
--	  found = true
--	  break
--	end
--  end
-- if not found then
--	for i=1, #self.fileobservers do
--    if self.fileobservers[i]:directory() == dir then
--	    self.fileobservers[i]:stopwatching()
--	    table.remove(self.fileobservers, i)
--		self.fileexplorer:setpath(dir)
--	    break	    
--    end  	  
--	end
--  end    
--end

return maincanvas