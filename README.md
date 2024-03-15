<h1>NewOutlookPatcher</h1>
<a href="https://discord.gg/gsPcfqHTD2"><img src="https://discordapp.com/api/guilds/1155912047897350204/widget.png?style=shield" alt="Join on Discord"></a>
<p>Disable ads and product placement in new Outlook for Windows app.</p>
<h2>Donate</h2>
<p>If you find this project essential to your daily life, please consider donating to support the development through the <a href="https://github.com/valinet/NewOutlookPatcher?sponsor">Sponsor</a> button at the top of this page, so that we can continue to keep supporting newer Windows builds.</p>
<h2>Features</h2>
<ul>
  <li>Disable ad as first item in e-mails list</li>
  <li>Disable lower left corner OneDrive banner</li>
  <li>Disable Word, Excel, PowerPoint, To Do, OneDrive, More apps icons</li>
  <li>Enable F12 Developer Tools</li>
</ul>
<div>
  <img src="https://github.com/valinet/NewOutlookPatcher/assets/6503598/0a6eb1eb-a9cc-4d3f-9632-01849963ae40" alt="Product image">
</div>
<h2>How to?</h2>
<ol type="1">
  <li>Download the <a href="https://github.com/valinet/NewOutlookPatcher/releases/latest/download/NewOutlookPatcher.exe">latest release</a>.</li>
  <li>Run <code>NewOutlookPatcher</code>. Outlook will also open automatically in the background.</li>
  <li>Customize the configuration by checking/unchecking individual items.</li>
  <li>Press <code>Patch</code>. The application will elevate itself, close Outlook, apply your setttings and restart Outlook for you.</li>
</ol>
<h2>Why is elevation required?</h2>
<p>New Outlook for Windows is installed in <code>Program Files - WindowsApps</code> which is write-protected for regular users, thus administrative access is required to place the patcher in that folder.</p>
<h2>Uninstalling</h2>
<p>Run <code>NewOutlookPatcher</code>. Uncheck all options. Press <code>Patch</code>. Done.</p>
<h2>Limitations</h2>
<ul>
  <li>Patcher does not start? Install <a href="https://dotnet.microsoft.com/en-us/download/dotnet/thank-you/runtime-desktop-8.0.3-windows-x64-installer">.NET Desktop Runtime 8</a>.</li>
  <li><a href="https://github.com/valinet/NewOutlookPatcher/issues/1">Patcher is x64-only at the moment.</a></li>
  <li><a href="https://github.com/valinet/NewOutlookPatcher/issues/2">Patcher does not read the current configuration at startup, but instead displays suggested default settings.</a></li>
</ul>
<h2>How it works?</h2>
<ul>
  <li>Everything is packed together in a tiny .NET 8-based executable. Required resources are extracted to a temporary folder at runtime.</li>
  <li>Patching Outlook (<code>olk.exe</code>) is done using the now classic <code>dxgi.dll</code> method, exploiting the DLL search order. The project contains a very clean C++ implementation of this technique. This works because the process is not protected, thus being able to load unsigned code.</li>
  <li>The actual patching is done by hooking WebView2 methods, in order to execute scripts that alter the CSS once the main interface loads.</li>
  <li>The main problem for this entire project was, believe it or not, copying the injector to Outlook's program folder. Being in the infamous <code>WindowsApps</code> folder which is thoroughly protected, after a ton of hours researching a user space solution, I resorted to exploiting CVE-2018-19320 to load an own compiled driver which does the copying in kernel space, thus bypassing any ACLs and other user space protections Windows imposes on that folder. CVE-2018-19320 is a technique where a known signed driver (in this case made by GIGABYTE) that allows for arbitrary kernel memory access via an IOCTL is loaded in the running kernel, and then the exposed IOCTL is used to temporarly disable DSE (driver signature enforcement) in order to further load our custom unsigned driver. DSE has to be enabled back as quickly as possible, as PatchGuard detects the change eventually and bug checks the machine if left like that.</li>
</ul>
<h2>Solution structure</h2>
<p>The Visual Studio solution is divided 4 projects:</p>
<ul>
  <li>gui: Contains user interface and unpacker logic, C# .NET 8.</li>
  <li>worker: Module that gets loaded by Outlook which injects custom JavaScript and CSS in the user interface.</li>
  <li>installer: Kernel mode driver which copies the worker to Outlook's program folder.</li>
  <li>loader: Loads the installer in the kernel.</li>
</ul>
<p>Successful compilation is only possible for x64 at the moment. Files packed in the final executable are always grabbed from the <code>Release</code> folder, beware when building in <code>Debug</code>.</p>
