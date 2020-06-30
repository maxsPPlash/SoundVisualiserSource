#pragma once
#include <string>
#include <Windows.h>
#include <comdef.h>

inline std::wstring StringToWide(std::string str)
{
	std::wstring wide_string(str.begin(), str.end());
	return wide_string;
}

inline void Log(std::string message)
{
	std::string error_message = "Error: " + message;
	MessageBoxA(NULL, error_message.c_str(), "Error", MB_ICONERROR);
}

inline void Log(HRESULT hr, std::string message)
{
	_com_error error(hr);
	std::wstring error_message = L"Error: " + StringToWide(message) + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
}

inline void Log(HRESULT hr, std::wstring message)
{
	_com_error error(hr);
	std::wstring error_message = L"Error: " + message + L"\n" + error.ErrorMessage();
	MessageBoxW(NULL, error_message.c_str(), L"Error", MB_ICONERROR);
}

inline float clamp(float n, float lower, float upper) {
  return max(lower, min(n, upper));
}

inline float smoothstep(float edge0, float edge1, float x) {
  // Scale, bias and saturate x to 0..1 range
  x = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
  // Evaluate polynomial
  return x * x * (3 - 2 * x);
}