<h1>NewOutlookPatcher</h1>
<a href="https://discord.gg/gsPcfqHTD2"><img src="https://discordapp.com/api/guilds/1155912047897350204/widget.png?style=shield" alt="Join on Discord"></a>
<p>Disable ads and product placement in new Outlook for Windows app.</p>
<p>Tested on:</p>
<ul>
  <li>Windows 10 Version 21H2 (OS Build 19044.4046)</li>
  <li>Windows 11 Version 23H2 (OS Build 22621.3296)</li>
</ul>
<h2>Donate</h2>
<p><a href="https://github.com/valinet/NewOutlookPatcher?sponsor">PayPal donations</a></p>
<h2>Features</h2>
<ul>
  <li>Disable ad as first item in e-mails list</li>
  <li>Disable lower left corner OneDrive banner</li>
  <li>Disable Word, Excel, PowerPoint, To Do, OneDrive, More apps icons</li>
  <li>Enable F12 Developer Tools</li>
</ul>
<div>
  <img src="https://github.com/valinet/NewOutlookPatcher/assets/6503598/8f60a3ee-b6de-4755-bccd-2dcc61386b6f" alt="Product image">
</div>
<h2>How to?</h2>
<ol type="1">
  <li>Download the <a href="https://github.com/valinet/NewOutlookPatcher/releases/latest/download/NewOutlookPatcher.exe">latest release</a>.</li>
  <li>Run <code>NewOutlookPatcher</code>. Outlook will also open automatically in the background.</li>
  <li>Customize the configuration by checking/unchecking individual items.</li>
  <li>Press <code>Install</code>. The application will elevate itself, close Outlook, apply your setttings and restart Outlook for you.</li>
</ol>
<h2>Why is elevation required?</h2>
The patcher requires administrative access in order to perform the following operations:
<ul>
  <li>Installing the patcher (<code>NewOutlookPatcher.dll</code>) in <code>C:\Windows\System32\</code> which is write-protected for regular users.</li>
  <li>Configuring New Outlook for Windows to load the patcher when it starts up using the registry (in <code>HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Image File Execution Options\olk.exe</code>).</li>
</ul>
<h2>Uninstalling</h2>
<p>Run <code>NewOutlookPatcher</code> and press the <code>Uninstall</code> button. Done.</p>
<h2>Known issues</h2>
<ul>
  <li>Patcher does not start? Install <a href="https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-desktop-8.0.3-windows-x64-installer">.NET Desktop Runtime 8</a>.</li>
  <li><a href="https://github.com/valinet/NewOutlookPatcher/issues/1">Patcher is x64-only at the moment.</a></li>
</ul>
<h2>How it works?</h2>
<ul>
  <li>Everything is packed together in a tiny .NET 8-based executable. Required resources are extracted to a temporary folder at runtime.</li>
  <li>New Outlook (<code>olk.exe</code>) is patched using a DLL that is injected in its process. The DLL is loaded using the <code>AppVerifier</code> infrastructure. The project contains a very clean C++ implementation of this technique. This works because the process is not protected, thus being able to load unsigned code.</li>
  <li>The actual patching is done by hooking WebView2 methods, in order to execute scripts that alter the CSS once the main interface loads.</li>
</ul>
<h2>Solution structure</h2>
<p>The Visual Studio solution is divided in 5 projects:</p>
<ul>
  <li>gui: Contains user interface and unpacker logic, C# .NET 8.</li>
  <li>worker: Module that gets loaded by Outlook which injects custom JavaScript and CSS in the user interface.</li>
</ul>
<p>Successful compilation is only possible for x64 at the moment. Files packed in the final executable are always grabbed from the <code>Release</code> folder, beware when building in <code>Debug</code>.</p>
