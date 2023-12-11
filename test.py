import re

def count_success_occurrences(text):
    pattern = r'\bsuccess\b(?![a-zA-Z])|\/\/[^\n]*'
    matches = re.findall(pattern, text)
    
    # 过滤掉注释行
    matches = [match for match in matches if not match.startswith('//')]
    
    return len(matches)

# 示例文本
sample_text = """
success
successful
// Comment line
success
another success
// Another comment line success // success
success
"""

result = count_success_occurrences(sample_text)
print(f'Total occurrences of "success": {result}')
