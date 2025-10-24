require 'win32ole'
require 'fiddle'
require 'fiddle/import'
require 'open3'

# Windows API module for serial port communication using fiddle
module WinAPI
  extend Fiddle::Importer
  dlload 'kernel32.dll'

  extern 'void* CreateFileA(char*, unsigned int, unsigned int, void*, unsigned int, unsigned int, void*)'
  extern 'int EscapeCommFunction(void*, unsigned int)'
  extern 'int CloseHandle(void*)'
end

# Constants for Windows API serial port operations
GENERIC_READ  = 0x80000000
GENERIC_WRITE = 0x40000000
OPEN_EXISTING = 3
SETDTR = 0x0005
SETRTS = 0x0003
CLRDTR = 0x0006
CLRRTS = 0x0004

def resetDevice(comPort, triggerLoader)
  port_name = "\\\\.\\#{comPort}\0"  # Null-terminated ANSI string. The slashes and dot are needed for Windows API compatibility

  handle = WinAPI.CreateFileA(
    port_name,
    GENERIC_READ | GENERIC_WRITE,
    0,
    0,              # lpSecurityAttributes (NULL)
    OPEN_EXISTING,
    0,
    0               # hTemplateFile (NULL)
  )
  
  if handle == 0 || handle == -1
    puts "Failed to open port"
    exit 1
  end

  WinAPI.EscapeCommFunction(handle, SETRTS)
  sleep 0.1

  if triggerLoader
    # Set the BOOT0 signal, to enter bootloader mode
    WinAPI.EscapeCommFunction(handle, CLRDTR)
    sleep 0.1
  end

  WinAPI.EscapeCommFunction(handle, CLRRTS)
  sleep 0.1

  WinAPI.CloseHandle(handle)
end

STDOUT.sync = true # make sure puts() output is flushed immediately

if File.file?(ARGV[0])
  target = ARGV[0]
else
  puts "Please provide the path to the file to be loaded (e.g. ELF or HEX) as the first argument.\n"
  puts "Adding \"internal\" as second argument will skip flashing data to external flash memory.\n"
  exit 1
end

only_load_internal = ARGV[1] == "internal"

# Find CH340 device and extract COM port
com_port = nil
wmi = WIN32OLE.connect("winmgmts://")
wmi.ExecQuery("SELECT * FROM Win32_PnPEntity WHERE Name LIKE '%(COM%'").each do |device|
  if device.Name.include?("CH340")
    # Extract the COM port number from the device name
    match = device.Name.match(/(COM\d*)/)
    com_port = match[1] if match
    break
  end
end

if !com_port
  puts "Device not found.\nMake sure the device is connected, the drivers are installed correctly, and try again.\nThe drivers are available at https://www.wch-ic.com/downloads/ch341ser_exe.html\n"
  exit 1
end

# Determine the path to STM32CubeProgrammer CLI
path_normal = "C:/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe"
path_CLT_tools = "#{ENV['STM32CLT_PATH']}/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe"

if File.file?(path_normal)
  cubeprgpath = path_normal
elsif File.file?(path_CLT_tools)
  cubeprgpath = path_CLT_tools
else
  puts "STM32CubeProgrammer CLI not found in default location: #{path_normal}."
  puts "Please install it there from https://www.st.com/en/development-tools/stm32cubeprog.html"
  exit 1
end

# Check that STM32CubeProgrammer version is new enough
cmdPrgVersion = "\"#{cubeprgpath}\" --version"
stdout, stderr, status = Open3.capture3(cmdPrgVersion)
if stdout =~ /STM32CubeProgrammer version:\s*([0-9.]+)/
  version = $1  # e.g., "2.20.0"
  if version =~ /^(\d+)\.(\d+)\.(\d+)$/
    major = $1.to_i
    minor = $2.to_i
    if major < 2 || minor < 18
      puts "Error STM32CubeProgrammer version needs to be at least v2.18.0 (v#{version} currently installed)."
      exit 1
    end
  else
    puts "STM32CubeProgrammer version format not recognized."
  end
else
  puts "STM32CubeProgrammer version not found in output."
end

puts "Rebooting the device into built-in bootloader"
resetDevice(com_port, true)

BASE_PROGRAMMER_COMMAND = "\"#{cubeprgpath}\" -c port=#{com_port} br=921600"

puts "Testing connection to bootloader on device"
success = system(BASE_PROGRAMMER_COMMAND)
if !success
  puts "\nFailed connecting to target. Try unplugging and replugging the device and try again.\n"
  puts "If the problem persists, please check that the option byte nBOOT_SEL is set to false. Refer to readme.md for more information."
  exit 1
end

if !only_load_internal
  puts "\nProgramming device with OpenBootloader to be able to program both internal and external flash memory.\n"
  success = system(BASE_PROGRAMMER_COMMAND + " -d ../Loading/OpenBootloader.hex --start 0x20003000")
  if !success
    exit 1
  end
end

puts "\nProgramming device with application (#{target})\n"
success = system(BASE_PROGRAMMER_COMMAND + " -d #{target}")
if !success && only_load_internal
  puts "\nThe device was programmed using the default bootloader. The internal flash was probably programmed correctly, if the error reported by CubeProgrammer is about the external section.\n"
end

puts "\nRebooting the device into the loaded application"
resetDevice(com_port, false)
