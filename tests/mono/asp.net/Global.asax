<script runat="server" language="c#">
	protected void Application_Start(Object sender, EventArgs e)
	{
		Console.WriteLine("Application_Start");
	}

	protected void Session_Start(Object sender, EventArgs e)
	{
		Console.WriteLine("Session_Start");
	}

	protected void Application_BeginRequest(Object sender, EventArgs e)
	{

	}

	protected void Application_EndRequest(Object sender, EventArgs e)
	{

	}

	protected void Application_AuthenticateRequest(Object sender, EventArgs e)
	{
		Console.WriteLine("Application_AuthenticateRequest");
	}

	protected void Application_Error(Object sender, EventArgs e)
	{
		Console.WriteLine("Application_Error");
	}

	protected void Session_End(Object sender, EventArgs e)
	{
		Console.WriteLine("Session_End");
	}

	protected void Application_End(Object sender, EventArgs e)
	{
		Console.WriteLine("Application_End");
	}
</script>
