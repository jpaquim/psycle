<%@ Page language="c#" 
    Codebehind="WebForm1.aspx.cs" 
    AutoEventWireup="false" 
    Inherits="WebApplication1.WebForm1" %>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN" >
<HTML>
	<HEAD>
		<title>WebForm1</title>
		<meta name="GENERATOR" Content="Microsoft Visual Studio .NET 7.1">
		<meta name="CODE_LANGUAGE" Content="C#">
		<meta name="vs_defaultClientScript" content="JavaScript">
		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
	</HEAD>
	<body MS_POSITIONING="GridLayout">
		<form id="Form1" method="post" runat="server">
			<asp:Button id="Button1" 
                style="Z-INDEX: 101; 
                        LEFT: 144px; 
                        POSITION: absolute; 
                        TOP: 64px" 
                runat="server"
				Text="Button">
            </asp:Button>
			<asp:TextBox id="TextBox1" 
                style="Z-INDEX: 102; 
                    LEFT: 256px; 
                    POSITION: absolute; 
                    TOP: 72px" 
                runat="server">
            </asp:TextBox>
		</form>
	</body>
</HTML>
