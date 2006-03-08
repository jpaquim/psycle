program SetupUtils;

uses
  SysUtils, Windows;

begin
  if ParamCount > 2 then
  case StrToInt(ParamStr(1)) of
    1: MoveFileEx(PChar(ParamStr(2)), PChar(ParamStr(3)), MOVEFILE_REPLACE_EXISTING);
  end;
end.
