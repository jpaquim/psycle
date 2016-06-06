-- psycle plugineditor (c) 2015 by psycledelics
-- File: maincanvas.lua
-- copyright 2015 members of the psycle project http://psycle.sourceforge.net
-- This source is free software ; you can redistribute it and/or modify it under
-- the terms of the GNU General Public License as published by the Free Software
-- Foundation ; either version 2, or (at your option) any later version.  

-- require('mobdebug').start()
-- local serpent = require("psycle.serpent")

local scintilla = require("psycle.ui.canvas.scintilla")
local lexer = require("psycle.ui.canvas.lexer")
local fileopen = require("psycle.ui.fileopen")
local filesave = require("psycle.ui.filesave")
local settings = require("settings")
local group = require("psycle.ui.canvas.group")
local canvas = require("psycle.ui.canvas")
local toolbar = require("psycle.ui.canvas.toolbar")
local toolicon = require("psycle.ui.canvas.toolicon")
local tabgroup = require("psycle.ui.canvas.tabgroup")
local splitter = require("psycle.ui.canvas.splitter")
local search = require("search")
local pluginexplorer = require("pluginexplorer")
local callstack = require("callstack")
local ornamentfactory = require("psycle.ui.canvas.ornamentfactory"):new()
local image = require("psycle.ui.image")
local signal = require("psycle.signal")
local item = require("psycle.ui.canvas.item")
local catcher = require("psycle.plugincatcher")
local serpent = require("psycle.serpent")
local createeditplugin = require("createeditplugin")
local filehelper = require("psycle.file")
local templateparser = require("templateparser")
local cfg = require("psycle.config"):new("PatternVisual")
local project = require("project")
local combobox = require("psycle.ui.canvas.combobox")
local machine = require("psycle.machine")
local machines = require("psycle.machines")
local modulepalette = require("modulepalette")
local moduledesigner = require("moduledesigner")
local objectinspector = require("objectinspector")
local text = require("psycle.ui.canvas.text")
local node = require("psycle.node")

local maincanvas = canvas:new()

function maincanvas:new()
  local c = canvas:new()  
  setmetatable(c, self)
  self.__index = self  
  c:init()
  return c
end

function node_iter(node)
  local i = {-1}
  local level_ = 1
  local n = node 
  return 
  { 
    level = function() return level_ - 1 end,
    next = function()
            i[level_] = i[level_] + 1
            if i[level_] == 0 then return n end
            if i[level_] <= n:size() then
              local child = n:at(i[level_])  
              if child:size() > 0 then
                level_ = level_ + 1
                i[level_] = 0
                n = child                
              end                 
              return child
            elseif level_ > 1 then
              level_ = level_ - 1              
              i[level_] = i[level_] + 1
              if i[level_] <= n:parent():size() then
                n = n:parent()
                return n:at(i[level_])
              end
            end
          end                                     
  }
end

function maincanvas:init()
  self.advancedview = false
  self:invalidatedirect()   
  self.togglecanvas = signal:new() 
  self:setornament(ornamentfactory:createfill(settings.canvas.colors.background))
  self:setupfiledialogs()
  self:inittoolbar()   
  self:createsearch()
  self:createcreateeditplugin()  
  self:createoutputs()
  splitter:new(self, splitter.HORZ)
  self.pluginexplorer = self:createpluginexplorer()
  splitter:new(self, splitter.VERT)
  self:createpagegroup()  
  if advancedview then
    self.objectinspector = objectinspector:new(self):setalign(item.ALRIGHT):setpos(0, 0, 200, 0)  
    splitter:new(self, splitter.VERT):setalign(item.ALRIGHT)  
  end            
end

function maincanvas:createcreateeditplugin()  
  self.createeditplugin = createeditplugin:new(self):hide():setalign(item.ALTOP)
  self.createeditplugin.doopen:connect(maincanvas.onopenplugin, self)
  self.createeditplugin.docreate:connect(maincanvas.oncreateplugin, self)
end

function maincanvas:createsearch()
  self.search = search:new(self):setpos(0, 0, 200, 200):hide():setalign(item.ALBOTTOM)
  self.search.dosearch:connect(maincanvas.onsearch, self)   
  self.search.dohide:connect(maincanvas.onsearchhide, self)
end

function maincanvas:createoutputs()
  self.outputs = tabgroup:new(self):setpos(0, 0, 0, 120):setalign(item.ALBOTTOM)  
  self.output = self:createoutput()
  self.outputs:addpage(self.output, "Output")  
  self.callstack = self:createcallstack()  
  --self.outputs:addpage(self.callstack, "Call stack")  
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
  local callstack = callstack:new(nil, self)
  callstack:setbackgroundcolor(0x2F2F2F) --settings.canvas.colors.background)
  callstack:settextcolor(settings.canvas.colors.foreground)
  return callstack
end

function maincanvas:createpagegroup()
  self.pages = tabgroup:new(self):setalign(item.ALCLIENT)
  self.pages.dopageclose:connect(maincanvas.onclosepage, self)
  self.newpagecounter = 1
end

function maincanvas:createpluginexplorer()
  local pluginexplorer = pluginexplorer:new(self):setpos(0, 0, 200, 0):setalign(item.ALLEFT)     
  pluginexplorer:setbackgroundcolor(0x2F2F2F) --settings.canvas.colors.background)
  pluginexplorer:settextcolor(settings.canvas.colors.foreground)  
  pluginexplorer.click:connect(maincanvas.onpluginexplorerclick, self)
  pluginexplorer.onremove:connect(maincanvas.onpluginexplorernoderemove, self)
  return pluginexplorer
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
  self.pluginexplorer:setfilepath(path)
  self.selecttoolbar.selectmachine:settext(plugininfo:name()):fls()
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
  lex:setstringcolor(settings.sci.lexer.stringcolor)
  lex:setwordcolor(settings.sci.lexer.wordcolor)
  lex:setfoldingcolor(settings.sci.lexer.foldingcolor)
  page:setlexer(lex)         
  page:setfont(settings.sci.lexer.font.name, settings.sci.lexer.font.size)
  page:setcaretcolor(0x939393)    
end

function maincanvas:createmodulepage()
  local page = self:createpage()  
  return page
end

function maincanvas:createpage()  
  local page = scintilla:new()  
  function page:onkeydown(ev)
    if ev:ctrlkey() then
      if ev:keycode() == 70 or ev:keycode() == 83 then
        ev:preventdefault()      
      end
    end  
  end    
  self:setlexer(page)
  local that = self
  function page:onfirstmodified()    
    local fname = self:filename()
    if fname ~= "" then    
      fname = fname:match("([^\\]+)$")      
      that.pages:setlabel(self, fname.."*")      
    else      
      that.pages:setlabel(self, "new"..self.pagecounter.."*")      
    end
  end   
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
    end
  end  
end

function maincanvas:openfromfile(fname, line)
  if not line  then line = 0 end
  local page = self:dopageexist(fname)
  if page ~= nil then
    self.pages:setactivepage(page)
  else    
    page = self:createpage()
    page:loadfile(fname)        
    local name = fname:match("([^\\]+)$")           
    if self.advancedview then
      local modulepagegroup = tabgroup:new():setautosize(false, false):disableclosebutton()
      modulepagegroup.tabbar:setalign(item.ALBOTTOM)
      modulepagegroup:addpage(page, "Source")
      local md = moduledesigner:new():setautosize(false, false)
      self.modulepalette:setdesigner(md)
      md:setobjectinspector(self.objectinspector)
      modulepagegroup:addpage(md, "Designer")    
      function modulepagegroup:loadfile(name)
         page:loadfile(fname)
      end
      function modulepagegroup:savefile(name)
         page:savefile(fname)
      end
      function modulepagegroup:filename()
        return page:filename()
      end
      function modulepagegroup:hasfile()
        return page:hasfile()
      end
      function modulepagegroup:gotoline(line)
        return page:gotoline(line)
      end
      self.pages:addpage(modulepagegroup, name)     
    else
      self.pages:addpage(page, name)     
    end    
  end  
  page:gotoline(line - 1)
end

function maincanvas:setcallstack(trace)
  for i=1, #trace do    
   -- self.callstack:add(trace[i])
  end 
  --self.callstack:autosize(3)
  --self.callstack:setdepth(1)  
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
      page:savefile(fname)
      fname = fname:match("([^\\]+)$")
      self.pages:setlabel(page, fname)
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
     end
  else
    self:savepage()    
    if  psycle.proxy.project:plugininfo() then
      local fname = psycle.proxy.project:plugininfo():dllname():match("([^\\]+)$"):sub(1, -5)    
      machine = machine:new(fname)
      local machines = machines:new()
      local pluginindex = machines:insert(machine)    
      psycle.proxy.project:setpluginindex(pluginindex)
      self:fillinstancecombobox()
      self:setpluginindex(pluginindex)
    end
  end
end

function maincanvas:inittoolbar()  
  self.tg = group:new(self):setautosize(false, true):setalign(item.ALTOP)--:setmargin(3, 3, 3, 3)    
  self.windowtoolbar = self:initwindowtoolbar():setalign(item.ALRIGHT)
  self.selecttoolbar = self:initselectplugintoolbar():setalign(item.ALLEFT)--:setmargin(4, 4, 4, 0)  
  self:initfiletoolbar():setalign(item.ALLEFT)--:setmargin(4, 4, 4, 0)
  self:initplaytoolbar():setalign(item.ALLEFT)--:setmargin(4, 4, 4, 0)  
  if self.advancedview then
    self.modulepalette = modulepalette:new(self.tg):setalign(item.ALCLIENT)
  end
  self:initstatus()
end

function maincanvas:initstatus()  
  local g = group:new(self.tg):setalign(item.ALRIGHT):setautosize(true, false)
  self.modifiedstatus = text:new(g):settext(""):setautosize(false, false):setpos(0, 0, 100, 0):setalign(item.ALLEFT):setmargin(0, 0, 5, 0)  
  text:new(g):settext("LINE"):setautosize(true, false):setalign(item.ALLEFT):setmargin(0, 0, 5, 0)
  self.linestatus = text:new(g):settext("1"):setautosize(false, false):setpos(0, 0, 50, 0):setalign(item.ALLEFT):setmargin(0, 0, 5, 0)
  text:new(g):settext("COL"):setautosize(true, false):setalign(item.ALLEFT):setmargin(0, 0, 5, 0)
  self.colstatus = text:new(g):settext("1"):setautosize(false, false):setpos(0, 0, 50, 0):setalign(item.ALLEFT):setmargin(0, 0, 5, 0)
  text:new(g):settext("INSERT"):setautosize(true, false):setalign(item.ALLEFT):setmargin(0, 0, 5, 0)
  self.insertstatus = text:new(g):settext("ON"):setautosize(false, false):setpos(0, 0, 30, 0):setalign(item.ALLEFT):setmargin(0, 0, 5, 0)  
end

function maincanvas:setwindowiconin()
  self.windowtoolbar.img = image:new()
                                :load(settings.picdir.."arrow_in.png")
                                :settransparent(0xFFFFFF)
end

function maincanvas:setwindowiconout()
   self.windowtoolbar.img = image:new()
                                 :load(settings.picdir.."arrow_out.png")
                                 :settransparent(0xFFFFFF)
end

function maincanvas:initwindowtoolbar()  
  local icon = toolicon:new(self.tg, settings.picdir.."arrow_out.png", 0xFFFFFF)  
  local that = self
  function icon:onclick()   
    that.togglecanvas:emit()
  end
  return icon
end

function maincanvas:initselectplugintoolbar(parent)
  local t = toolbar:new(self.tg)
  t.selectmachine = toolicon:new(t):settext("No Plugin Loaded"):setpos(0, 0, 100, 20)  
  local that = self
  function t.selectmachine:onclick()
    local catcher = catcher:new()
    local infos = catcher:infos()    
    that.createeditplugin:show()
    that:updatealign()    
  end  
  return t
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
  self.cbx = combobox:new(parent):setautosize(false, false):setpos(0, 0, 100, 20):setalign(item.ALLEFT)
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
  local that = self    
  function inew:onclick() that:createnewpage() end  
  function iopen:onclick() that.fileopen:show() end
  function isave:onclick() that:savepage() end
  return t
end

function maincanvas:initplaytoolbar()  
  local t = toolbar:new(self.tg)
  local istart = toolicon:new(t, settings.picdir.."play.png", 0xFFFFFF)  
  local that = self
  function istart:onclick() that:playplugin() end
  self:createinstanceselect(t)
  return t
end

function maincanvas:oncallstackclick(info)
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

function maincanvas:onsearch(searchtext, dir, case, wholeword, regexp)    
  local page = self.pages:activepage()
  if page then  
    local cpmin, cpmax = 0, 0
    if dir == search.DOWN then      
      cpmin = page:selectionstart()
      cpmax = page:length()
      if page:hasselection() then       
       cpmin = cpmin + 1
      end
    else
      cpmin = page:selectionstart()
      cpmax = 0
      if page:hasselection() then 
        cpmax = cpmax - 1
      end
    end        
    page:setfindmatchcase(case)
    page:setfindwholeword(wholeword)
    page:setfindregexp(regexp)    
    local line, cpselstart, cpselend = page:findtext(searchtext, cpmin, cpmax)    
    if line ~= -1 then
      page:setsel(cpselstart, cpselend)      
    end
  end
end

function maincanvas:displaysearch(ev)
  self.search:show():onfocus()
  self:updatealign()
end

function maincanvas:oncreateplugin(pluginname, templatepath)
  local env = { vendor = "psycle" }
  env.pluginname = pluginname  
  templateparser.work(cfg:luapath().."\\plugineditor\\templates\\pluginregister.lu$",
                      cfg:luapath().."\\"..pluginname..".lua",
                      env)  
  filehelper.mkdir(pluginname)
  templateparser.work(templatepath,
                      cfg:luapath().."\\"..pluginname.."\\machine.lua",
                      env)
  self:openplugin(pluginname.."\\machine", pluginname)
  local catcher = catcher:new():rescannew()
  local infos = catcher:infos()        
  for i=1, #infos do       
    if infos[i]:type() == machine.MACH_LUA and
      infos[i]:name():lower() == pluginname:lower() then      
      psycle.proxy.project = project:new():setplugininfo(infos[i])
      break;     
    end
  end    
end

function maincanvas:onopenplugin(pluginpath, pluginname, info)  
  self:openplugin(pluginpath, pluginname, info)
  psycle.proxy.project = project:new():setplugininfo(info)
  self:fillinstancecombobox()  
end

function findlast(haystack, needle)
  local i=haystack:match(".*"..needle.."()")
  if i==nil then return else return i end
end

function maincanvas:openplugin(pluginpath, pluginname, plugininfo)  
  self:preventfls()
  self:closealltabs()      
  self.pluginexplorer:setfilepath(cfg:luapath().."\\"..pluginpath:sub(1, pluginpath:find("\\")))
  self.selecttoolbar.selectmachine:settext(pluginname)
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
     self:openfromfile(ev.path..ev.filename, 0)
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
end

function maincanvas:onsearchhide()
  if self.pages:activepage() then
    self.pages:activepage():setfocus()
  end
end

function maincanvas:onclosepage(ev)  
  if ev.page:modified() and psycle.confirm("Do you want to save changes to "..ev.page:filename().."?") then
    self:savepage()
  end
end

return maincanvas